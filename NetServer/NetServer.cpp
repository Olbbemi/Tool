#include "Precompile.h"
#include "NetServer.h"

#include "Protocol/Define.h"
#include "Profile/Profile.h"
#include "Serialize/Serialize.h"
#include "RingBuffer/RingBuffer.h"

#include <process.h>
#include <strsafe.h>

#define INDEX_VALUE 65535
#define IS_ALIVE 1
#define IS_NOT_ALIVE 0

using namespace Olbbemi;

/**-----------------------------------------------------------------------------------------------------------
  * F1 키가 눌러지면 호출되는 함수로 서버를 시작하기위한 리소스를 생성
  * 배열을 이용하여 모든 락을 제거한 구조 사용 [ interlock 함수는 제외 ]
  * m_session_list 구조체 내의 session_info 변수 [ 0: 해당 배열의 사용여부를 확인, 1: 해당 배열의 IO_count 정보 ]
  *-----------------------------------------------------------------------------------------------------------*/
bool C_NetServer::M_Start(bool pa_is_nagle_on, BYTE pa_work_count, TCHAR* pa_ip, WORD pa_port, DWORD pa_max_session, BYTE pa_packet_code, BYTE pa_first_key, BYTE pa_second_key)
{
	TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");

	m_total_accept = 0;	v_network_session_count = 0;
	v_accept_tps = 0;	v_network_tps = 0;

	m_session_count = 0;
	m_is_nagle_on = pa_is_nagle_on;			m_workthread_count = pa_work_count;
	StringCchCopy(m_ip, 17, pa_ip);			m_port = pa_port;
	m_max_session_count = pa_max_session;

	m_packet_code = pa_packet_code; 
	m_first_key = pa_first_key; m_second_key = pa_second_key;

	m_probable_index = new C_LFStack<WORD>;

	m_thread_handle = new HANDLE[pa_work_count + 1];
	m_session_list = new ST_Session[pa_max_session];
	for (int i = 1; i < pa_max_session; i++)
	{
		m_session_list[i].v_session_info[1] = 0;
		m_session_list[i].v_session_info[0] = IS_NOT_ALIVE;
		m_probable_index->M_Push(i);
	}

	M_CreateIOCPHandle(m_iocp_handle);
	if (m_iocp_handle == NULL)
	{
		ST_Log lo_log({ "Create IOCP Handle Error Code: " + to_string(WSAGetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);

		return false;
	}

	m_thread_handle[0] = (HANDLE)_beginthreadex(NULL, 0, M_AcceptThread, this, 0, nullptr);
	if (m_thread_handle[0] == 0)
	{
		ST_Log lo_log({ "Create AcceptThread Error Code: " + to_string(WSAGetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);

		return false;
	}

	for (int i = 1; i < pa_work_count + 1; ++i)
	{
		m_thread_handle[i] = (HANDLE)_beginthreadex(NULL, 0, M_WorkerThread, this, 0, nullptr);
		if (m_thread_handle[i] == 0)
		{
			ST_Log lo_log({ "Create WorkerThread Error Code: " + to_string(WSAGetLastError()) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);

			return false;
		}
	}

	return true;
}

/**---------------------------------------------------------------------------------
  * F2 키가 눌러지면 호출되는 함수로 서버를 종료하기 위해 리소스 해제
  * 서버에서 종료하므로 연결되어 있는 모든 클라이언트에게 shutdown 을 통하여 종료를 알림
  * 모든 클라이언트가 종료됨을 확인한 뒤 각 쓰레드 종료 및 리소스 해제
  *---------------------------------------------------------------------------------*/
void C_NetServer::M_Stop()
{
	closesocket(m_listen_socket);
	for (int i = 0; i < m_max_session_count; ++i)
	{
		if (m_session_list[i].v_session_info[0] == IS_ALIVE)
			shutdown(m_session_list[i].socket, SD_BOTH);
	}

	while (1)
	{
		bool lo_flag = false;
		for (int i = 0; i < m_max_session_count; i++)
		{
			if (m_session_list[i].v_session_info[1] != 0)
			{
				lo_flag = true;
				break;
			}
		}

		if (lo_flag == false)
			break;
	}

	VIR_OnClose();
	PostQueuedCompletionStatus(m_iocp_handle, 0, 0, 0);

	DWORD lo_wait_value = WaitForMultipleObjects(m_workthread_count + 1, m_thread_handle, TRUE, INFINITE);
	if (lo_wait_value == WAIT_FAILED)
	{
		TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");
		ST_Log lo_log({ "WaitForMulti Error Code: " + to_string(WSAGetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}

	for (int i = 0; i < m_workthread_count + 1; ++i)
		CloseHandle(m_thread_handle[i]);

	delete[] m_thread_handle;	delete[] m_session_list;
	delete m_probable_index;

	_tprintf(_TEXT("Stop Success!!!\n"));
}

/**--------------------------
  * IOCP 핸들을 얻기 위한 함수
  *--------------------------*/
void C_NetServer::M_CreateIOCPHandle(HANDLE& pa_handle)
{
	pa_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

/**-------------------------------------
  * 소켓 정보를 IOCP 에 등록하기 위한 함수
  *-------------------------------------*/
bool C_NetServer::M_MatchIOCPHandle(SOCKET pa_socket, ST_Session* pa_session)
{
	HANDLE lo_handle_value = CreateIoCompletionPort((HANDLE)pa_socket, m_iocp_handle, (ULONG_PTR)pa_session, 2);
	if (lo_handle_value == NULL)
		return false;

	return true;
}

/**------------------------
  * Accept 를 전담할 쓰레드
  *------------------------*/
unsigned int __stdcall C_NetServer::M_AcceptThread(void* pa_argument)
{
	return ((C_NetServer*)pa_argument)->M_Accept();
}

/**---------------------------
  * 패킷 송수신을 전담할 쓰레드
  *---------------------------*/
unsigned int __stdcall C_NetServer::M_WorkerThread(void* pa_argument)
{
	return ((C_NetServer*)pa_argument)->M_PacketProc();
}

/**-----------------------------------------------------------------------------------------------
  * Accept 쓰레드에서 호출한 콜백 함수
  * 연결이 성공적으로 이루어지면 해당 클라이언트를 저장할 배열인덱스를 락프리 스택을 이용하여 얻어옴
  * 해당 배열에 리소스 할당 및 소켓을 IOCP에 등록
  * Stop 함수에서 listen_socket 을 종료하면 accept 에서 에러 발생하여 해당 쓰레드를 종료하는 방법 이용
  * 쓰레드가 종료되기 전 직렬화 버퍼의 메모리 누수를 막기 위해 C_Serialize::S_Terminate() 함수 호출
  *-----------------------------------------------------------------------------------------------*/
unsigned int C_NetServer::M_Accept()
{
	TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");
	
	WSADATA lo_wsadata;
	SOCKET lo_client_socket;
	SOCKADDR_IN lo_server_address, lo_client_address;
	int lo_error_check, lo_len = sizeof(lo_server_address);

	WSAStartup(MAKEWORD(2, 2), &lo_wsadata);

	m_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listen_socket == INVALID_SOCKET)
	{
		ST_Log lo_log({ "Listen Socket Error Code: " + to_string(WSAGetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);

		return 0;
	}

	ZeroMemory(&lo_server_address, sizeof(lo_server_address));
	lo_server_address.sin_family = AF_INET;
	WSAStringToAddress(m_ip, AF_INET, NULL, (SOCKADDR*)&lo_server_address, &lo_len);
	WSAHtons(m_listen_socket, m_port, &lo_server_address.sin_port);

	lo_error_check = bind(m_listen_socket, (SOCKADDR*)&lo_server_address, sizeof(lo_server_address));
	if (lo_error_check == SOCKET_ERROR)
	{
		ST_Log lo_log({ "Socket Bind Error Code: " + to_string(WSAGetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);

		return 0;
	}

	lo_error_check = listen(m_listen_socket, SOMAXCONN);
	if (lo_error_check == SOCKET_ERROR)
	{
		ST_Log lo_log({ "Listen Error Code: " + to_string(WSAGetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);

		return 0;
	}

	while (1)
	{
		lo_client_socket = accept(m_listen_socket, (SOCKADDR*)&lo_client_address, &lo_len);
		if (lo_client_socket == INVALID_SOCKET)
		{
			int lo_error = WSAGetLastError();
			if (lo_error == WSAEINTR || lo_error == WSAENOTSOCK)
			{
				C_Serialize::S_Terminate();
				break;
			}
				
			ST_Log lo_log({ "Socket Accept Error Code: " + to_string(lo_error) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			closesocket(lo_client_socket);

			continue;
		}

		bool  lo_connect_check, lo_iocp_check;
		TCHAR lo_confirm_ip[17];	WORD lo_confirm_port = 0, lo_avail_index;

		WSAStringToAddress(lo_confirm_ip, AF_INET, NULL, (SOCKADDR*)&lo_client_address, &lo_len);
		WSANtohs(lo_client_socket, lo_confirm_port, &lo_client_address.sin_port);

		lo_connect_check = VIR_OnConnectionRequest(lo_confirm_ip, lo_confirm_port);
		if (lo_connect_check == false)
		{
			wstring lo_buffer = lo_confirm_ip;		string lo_via_buffer(lo_buffer.begin(), lo_buffer.end());

			ST_Log lo_log({ "Ip: " + lo_via_buffer + ", Port: " + to_string(lo_confirm_port) + "Can't Accpet" });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			closesocket(lo_client_socket);

			continue;
		}

		if (m_probable_index->M_GetUseCount() == 0)
		{
			closesocket(lo_client_socket);
			continue;
		}

		lo_avail_index = m_probable_index->M_Pop();
		m_session_list[lo_avail_index].session_id = ((m_session_count << 16) | lo_avail_index);
		InterlockedIncrement(&m_session_list[lo_avail_index].v_session_info[1]);
		InterlockedExchange(&m_session_list[lo_avail_index].v_session_info[0], IS_ALIVE);

		m_session_list[lo_avail_index].socket = lo_client_socket;	m_session_list[lo_avail_index].v_send_count = 0;
		m_session_list[lo_avail_index].recvOver = new OVERLAPPED;	m_session_list[lo_avail_index].sendOver = new OVERLAPPED;
		m_session_list[lo_avail_index].recvQ = new C_RINGBUFFER;	m_session_list[lo_avail_index].sendQ = new C_LFQueue<C_Serialize*>;
		m_session_list[lo_avail_index].v_send_flag = FALSE;
		m_session_count++;

		lo_iocp_check = M_MatchIOCPHandle(lo_client_socket, &m_session_list[lo_avail_index]);
		if (lo_iocp_check == false)
		{
			ST_Log lo_log({ "IOCP Matching Error Code: " + to_string(WSAGetLastError()) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			M_Release(m_session_list[lo_avail_index].session_id);

			continue;
		}

		VIR_OnClientJoin(m_session_list[lo_avail_index].session_id, lo_confirm_ip, lo_confirm_port);
		M_RecvPost(&m_session_list[lo_avail_index]);

		m_total_accept++;
		InterlockedIncrement(&v_network_session_count);
		InterlockedIncrement(&v_accept_tps);

		LONG interlock_value = InterlockedDecrement(&m_session_list[lo_avail_index].v_session_info[1]);
		if (interlock_value == 0)
			M_Release(m_session_list[lo_avail_index].session_id);
		else if (interlock_value < 0)
		{
			ST_Log lo_log({ "IO Count is Nagative: " + to_string(interlock_value) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
		}
	}

	WSACleanup();
	return 1;
}

/**---------------------------------------------------------------------------------------------
  * Worker 쓰레드에서 호출한 콜백 함수
  * IOCP 를 이용하여 모든 클라이언트의 패킷 송수신을 처리
  * 각 세션마다 IO_count 를 소유하고 있으며 이 값이 0이 되는 경우에만 세션 종료
  * 외부에서 PostQueuedCompletionStatus 함수를 호출하면 해당 쓰레드가 종료되는 구조
  * 쓰레드가 종료되기 전 직렬화 버퍼의 메모리 누수를 막기 위해 C_Serialize::S_Terminate() 함수 호출
  *---------------------------------------------------------------------------------------------*/
unsigned int C_NetServer::M_PacketProc()
{
	TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");

	while (1)
	{
		bool lo_recvpost_value = true;
		char lo_sendpost_value = 0;

		DWORD lo_transfered = 0;
		ST_Session* lo_session = nullptr;
		OVERLAPPED* lo_overlap = nullptr;

		GetQueuedCompletionStatus(m_iocp_handle, &lo_transfered, (ULONG_PTR*)&lo_session, &lo_overlap, INFINITE);

		if (lo_transfered == 0 && lo_session == nullptr && lo_overlap == nullptr)
		{
			C_Serialize::S_Terminate();
			PostQueuedCompletionStatus(m_iocp_handle, 0, 0, 0);

			break;
		}
		else if (lo_overlap == nullptr)
		{
			ST_Log lo_log({ "GQCS Error Code: " + to_string(WSAGetLastError()) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
		}
		else if (lo_transfered != 0 && lo_overlap == lo_session->recvOver)
		{
			bool lo_check = true;
			lo_session->recvQ->M_MoveRear(lo_transfered);

			while (1)
			{
				int lo_size = 0, lo_recv_size;
				ST_Header lo_header;
				C_Serialize* lo_serialQ = C_Serialize::S_Alloc();
				
				lo_recv_size = lo_session->recvQ->M_GetUseSize();
				if (lo_recv_size < NET_HEAD_SIZE)
				{
					C_Serialize::S_Free(lo_serialQ);
					break;
				}
					
				lo_session->recvQ->M_Peek((char*)&lo_header, NET_HEAD_SIZE, lo_size);
				if (lo_recv_size < lo_header.len + NET_HEAD_SIZE)
				{
					C_Serialize::S_Free(lo_serialQ);
					break;
				}
					
				lo_session->recvQ->M_Dequeue(lo_serialQ->M_GetBufferPtr(), lo_header.len + NET_HEAD_SIZE);
				lo_serialQ->M_MoveRear(lo_header.len + NET_HEAD_SIZE);

				lo_check = Decode(lo_serialQ);
				if (lo_check == false)
				{
					C_Serialize::S_Free(lo_serialQ);
					break;
				}

				InterlockedIncrement(&v_network_tps);

				C_Serialize::S_AddReference(lo_serialQ);
				VIR_OnRecv(lo_session->session_id, lo_serialQ);
				C_Serialize::S_Free(lo_serialQ);
			}

			if(lo_check == true)
				lo_recvpost_value = M_RecvPost(lo_session);
		}
		else if (lo_overlap == lo_session->sendOver)
		{
			ULONG lo_count = lo_session->v_send_count;
			for (int i = 0; i < lo_count; ++i)
			{
				C_Serialize::S_Free(lo_session->store_buffer[i]);
				lo_session->store_buffer[i] = nullptr;
			}

			InterlockedExchangeSubtract(&lo_session->v_send_count, lo_count);
			InterlockedExchange(&lo_session->v_send_flag, FALSE);
			lo_sendpost_value = M_SendPost(lo_session);
		}

		if (lo_recvpost_value == true && lo_sendpost_value != -1)
		{
			LONG interlock_value = InterlockedDecrement(&lo_session->v_session_info[1]);
			if (interlock_value == 0)
				M_Release(lo_session->session_id);
			else if (interlock_value < 0)
			{
				ST_Log lo_log({ "IO Count is Nagative: " + to_string(interlock_value) });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}
		}
	}

	return 0;
}

/**----------------------------------------------------------------------------------------------------------------------------------
  * 인코딩할 때 사용할 키 값 및 체크 코드를 Start 함수에서 얻어옴 
  * 패킷을 전송하기 전 키 값으로 암호화 후 서버 프로토콜에 맞는 헤더 삽입
  * 동일 패킷에 대해서 암호화를 할 필요가 없으므로 flag를 이용하여 이미 인코딩된 패킷인지 확인
  * 주어진 암호화 방식에는 문제점이 있음 [ 난수로 할당 받는 암호 키까지 고정 키로 암호화를 함으로써 고정키가 달라도 암호화가 해제 되어버림 ]
  *----------------------------------------------------------------------------------------------------------------------------------*/
void C_NetServer::Encode(C_Serialize* pa_serialQ)
{
	if (pa_serialQ->m_is_encode == true)
		return;

	pa_serialQ->m_is_encode = true;

	char* serialQ_ptr = pa_serialQ->M_GetBufferPtr();
	BYTE lo_mixture_key = m_first_key ^ m_second_key;
	ST_Header lo_header;

	lo_header.code = m_packet_code;
	lo_header.len = pa_serialQ->M_GetUsingSize();
	lo_header.xor_code = (rand() & 255);

	lo_header.checksum = 0;
	for (int i = 0; i < lo_header.len; i++)
		lo_header.checksum += serialQ_ptr[i];
	
	lo_header.xor_code ^= lo_mixture_key;
	lo_header.checksum ^= lo_header.xor_code;

	for (int i = 0; i < lo_header.len; i++)
		serialQ_ptr[i] ^= lo_header.xor_code;

	pa_serialQ->M_NetMakeHeader((char*)&lo_header, sizeof(ST_Header));
}

/**----------------------------------------------------------------------------
  * 수신한 패킷을 고정 키 값으로 해제 후 Checksum 비교
  * 컨텐츠 처리부는 서버 프로토콜이 필요없으므로 이 정보는 제외하고 전송하도록 설정
  *----------------------------------------------------------------------------*/
bool C_NetServer::Decode(C_Serialize* pa_serialQ)
{
	BYTE lo_mixture_key = m_first_key ^ m_second_key;
	BYTE lo_sum = 0;
	ST_Header* lo_header = (ST_Header*)pa_serialQ->M_GetBufferPtr();
	if (lo_header->code != m_packet_code)
		return false;
	
	pa_serialQ->M_MoveFront(NET_HEAD_SIZE);
	char* serialQ_ptr = (char*)((char*)lo_header + NET_HEAD_SIZE);

	lo_header->checksum ^= lo_header->xor_code;
	for (int i = 0; i < lo_header->len; i++)
		serialQ_ptr[i] ^= lo_header->xor_code;

	for (int i = 0; i < lo_header->len; i++)
		lo_sum += (BYTE)serialQ_ptr[i];
	
	if (lo_header->checksum != lo_sum)
		return false;
	
	return true;
}

/**---------------------------------------------------------------------------------------------------------
  * 컨텐츠에서 패킷을 보내거나 세션연결을 종료하기 위해 사용되는 함수내에 호출
  * 해당 세션의 IO_count를 증가
  * 해당 세션이 이미 Release 단계에 들어간 경우에는 패킷 전송 및 세션 종료를하면 안되므로 interlock을 통하여 확인
  * 이미 Release 단계로 들어갔다면 0을 반환 [ 세션 배열에서 0은 사용하지 않음 ]
  *---------------------------------------------------------------------------------------------------------*/
bool C_NetServer::SessionAcquireLock(LONG64 pa_session_id, WORD& pa_index)
{
	WORD lo_index = (WORD)(pa_session_id & INDEX_VALUE);
	pa_index = lo_index;

	InterlockedIncrement(&m_session_list[lo_index].v_session_info[1]);
	if (m_session_list[lo_index].v_session_info[0] != IS_ALIVE || m_session_list[lo_index].session_id != pa_session_id)
		return false;

	return true;
}

/**--------------------------------------------------------------------------------
  * SessionAcquireLock 함수와 한 쌍을 이루는 함수
  * 위 함수를 호출한 위치의 마지막에는 SesseionAcquireUnlock 함수를 꼭 호출해주어야 함
  *--------------------------------------------------------------------------------*/
void C_NetServer::SessionAcquireUnlock(LONG64 pa_session_id, WORD pa_index)
{
	LONG lo_count = InterlockedDecrement(&m_session_list[pa_index].v_session_info[1]);
	if (lo_count == 0)
		M_Release(pa_session_id);
	else if (lo_count < 0)
	{
		TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");

		ST_Log lo_log({ "IO Count is Nagative: " + to_string(lo_count) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}
}

/**---------------------------------------------------------
  * 컨텐츠나 네트워크에서 보낼 패킷이 있을 때마다 호출하는 함수
  *---------------------------------------------------------*/
void C_NetServer::M_SendPacket(LONG64 pa_session_id, C_Serialize* pa_packet)
{
	bool lo_check;
	WORD lo_index;
	
	lo_check = SessionAcquireLock(pa_session_id, lo_index);
	if (lo_check == false)
	{
		C_Serialize::S_Free(pa_packet);

		LONG lo_count = InterlockedDecrement(&m_session_list[lo_index].v_session_info[1]);
		if (lo_count == 0)
			M_Release(pa_session_id);
		else if (lo_count < 0)
		{
			TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");

			ST_Log lo_log({ "IO Count is Nagative: " + to_string(lo_count) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
		}
		
		return;
	}
		
	Encode(pa_packet);

	C_Serialize::S_AddReference(pa_packet);
	m_session_list[lo_index].sendQ->M_Enqueue(pa_packet);
	
	M_SendPost(&m_session_list[lo_index]);
	C_Serialize::S_Free(pa_packet);

	SessionAcquireUnlock(pa_session_id, lo_index);
}

/**-----------------------------------------------------------------------------------------------------------------------------
  * 컨텐츠에서 종료를 원할 때 호출하는 함수
  * 해당 세션이 Release 단계에 들어가면 OnRelease 함수가 호출되므로 해당 함수 호출하기 전 컨텐츠에서 해당 세션을 미리 정리할 필요 없음
  * 서버에서 먼저 종료를 요청하는 것이므로 closesocket이 아닌 shutdown 함수를 호출해야 함
  *-----------------------------------------------------------------------------------------------------------------------------*/
void C_NetServer::M_Disconnect(LONG64 pa_session_id)
{
	bool lo_check;
	WORD lo_index;
	
	lo_check = SessionAcquireLock(pa_session_id, lo_index);
	if (lo_check == false)
	{
		LONG lo_count = InterlockedDecrement(&m_session_list[lo_index].v_session_info[1]);
		if (lo_count == 0)
			M_Release(pa_session_id);
		else if (lo_count < 0)
		{
			TCHAR lo_action[] = _TEXT("NetServer"), lo_server[] = _TEXT("ChatServer");

			ST_Log lo_log({ "IO Count is Nagative: " + to_string(lo_count) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
		}

		return;
	}
		
	shutdown(m_session_list[lo_index].socket, SD_BOTH);
	SessionAcquireUnlock(pa_session_id, lo_index);
}

/**---------------------------------------
  * 각 세션에 대한 수신을 처리하기 위한 함수
  *---------------------------------------*/
bool C_NetServer::M_RecvPost(ST_Session* pa_session)
{
	WSABUF lo_wsabuf[2];
	DWORD flag = 0, size = 0, lo_buffer_count = 1, lo_unuse_size = pa_session->recvQ->M_GetUnuseSize(), lo_linear_size = pa_session->recvQ->M_LinearRemainRearSize();

	if (lo_unuse_size == lo_linear_size)
	{
		lo_wsabuf[0].buf = pa_session->recvQ->M_GetRearPtr();
		lo_wsabuf[0].len = lo_linear_size;
	}
	else
	{
		lo_wsabuf[0].buf = pa_session->recvQ->M_GetRearPtr();	lo_wsabuf[0].len = lo_linear_size;
		lo_wsabuf[1].buf = pa_session->recvQ->M_GetBasicPtr();	lo_wsabuf[1].len = lo_unuse_size - lo_linear_size;
		lo_buffer_count++;
	}

	ZeroMemory(pa_session->recvOver, sizeof(OVERLAPPED));
	InterlockedIncrement(&pa_session->v_session_info[1]);

	int lo_recv_value = WSARecv(pa_session->socket, lo_wsabuf, lo_buffer_count, &size, &flag, pa_session->recvOver, NULL);
	if (lo_recv_value == SOCKET_ERROR)
	{
		int lo_error_check = WSAGetLastError();
		if (lo_error_check != WSA_IO_PENDING)
		{
			LONG interlock_value = InterlockedDecrement(&pa_session->v_session_info[1]);
			if (interlock_value == 0)
			{
				M_Release(pa_session->session_id);
				return false;
			}
		}
	}

	return true;
}

/**---------------------------------------
  * 각 세션에 대한 송신을 처리하기 위한 함수
  *---------------------------------------*/
char C_NetServer::M_SendPost(ST_Session* pa_session)
{
	WSABUF lo_wsabuf[500];
	DWORD lo_size = 0, lo_buffer_count = 0;

	while (1)
	{
		if (InterlockedCompareExchange(&pa_session->v_send_flag, TRUE, FALSE) == TRUE)
			return 1;

		int lo_use_count = pa_session->sendQ->M_GetUseCount();
		if (lo_use_count == 0)
		{
			InterlockedExchange(&pa_session->v_send_flag, FALSE);

			if (pa_session->sendQ->M_GetUseCount() != 0)
				continue;
			else
				return 0;
		}

		if (lo_use_count > 500)
			lo_use_count = 500;

		for (int i = 0; i < lo_use_count; ++i)
		{
			pa_session->sendQ->M_Dequeue(pa_session->store_buffer[i]);

			lo_wsabuf[lo_buffer_count].buf = pa_session->store_buffer[i]->M_GetBufferPtr();
			lo_wsabuf[lo_buffer_count++].len = pa_session->store_buffer[i]->M_GetUsingSize();

			InterlockedIncrement(&pa_session->v_send_count);
		}

		ZeroMemory(pa_session->sendOver, sizeof(OVERLAPPED));
		InterlockedIncrement(&pa_session->v_session_info[1]);

		int lo_wsasend_value = WSASend(pa_session->socket, lo_wsabuf, lo_buffer_count, &lo_size, 0, pa_session->sendOver, NULL);
		if (lo_wsasend_value == SOCKET_ERROR)
		{
			int lo_error_check = WSAGetLastError();
			if (lo_error_check != WSA_IO_PENDING)
			{
				LONG lo_interlock_value = InterlockedDecrement(&pa_session->v_session_info[1]);
				if (lo_interlock_value == 0)
				{
					M_Release(pa_session->session_id);
					return -1;
				}
			}
		}

		return 0;
	}
}

/**-------------------------------------------------------------------------------
  * 세션을 종료하기 위해 호출되는 함수 [ 해당 세션이 사용한 모든 리소스 해제 및 반환 ]
  *-------------------------------------------------------------------------------*/
void C_NetServer::M_Release(LONG64 pa_session_id)
{
	WORD lo_index = (WORD)(pa_session_id & INDEX_VALUE);
	
	if (InterlockedCompareExchange64((LONG64*)m_session_list[lo_index].v_session_info, IS_NOT_ALIVE, IS_ALIVE) != IS_ALIVE)
		return;

	InterlockedDecrement(&v_network_session_count);

	int lo_count = m_session_list[lo_index].v_send_count;
	for (int i = 0; i < lo_count; i++)	
		C_Serialize::S_Free(m_session_list[lo_index].store_buffer[i]);
	
	int lo_sendQ_count = m_session_list[lo_index].sendQ->M_GetUseCount();
	if (lo_sendQ_count != 0)
	{
		for (int i = 0; i < lo_sendQ_count; i++)
		{
			m_session_list[lo_index].sendQ->M_Dequeue(m_session_list[lo_index].store_buffer[i]);
			C_Serialize::S_Free(m_session_list[lo_index].store_buffer[i]);
		}
	}

	delete m_session_list[lo_index].recvOver;	delete m_session_list[lo_index].sendOver;
	delete m_session_list[lo_index].recvQ;		delete m_session_list[lo_index].sendQ;

	m_session_list[lo_index].recvOver = nullptr; m_session_list[lo_index].sendOver = nullptr;
	m_session_list[lo_index].recvQ = nullptr;	 m_session_list[lo_index].sendQ = nullptr;

	closesocket(m_session_list[lo_index].socket);
	m_session_list[lo_index].v_session_info[0] = false;

	m_probable_index->M_Push(lo_index);
	VIR_OnClientLeave(pa_session_id);
}

//============================================================================================================

/**---------------------------------
  * 서버에서 관제용으로 출력하는 함수
  *---------------------------------*/

LONG C_NetServer::M_TLSPoolAllocCount()
{
	return C_Serialize::S_TLSAllocCount();
}

LONG C_NetServer::M_TLSPoolChunkCount()
{
	return C_Serialize::S_TLSChunkCount();
}

LONG C_NetServer::M_TLSPoolNodeCount()
{
	return C_Serialize::S_TLSNodeCount();
}

LONG C_NetServer::M_StackAllocCount()
{
	return m_probable_index->M_GetAllocCount();
}

LONG C_NetServer::M_StackUseCount()
{
	return m_probable_index->M_GetUseCount();
}

LONG64 C_NetServer::M_TotalAcceptCount()
{
	return m_total_accept;
}

LONG C_NetServer::M_AcceptTPS()
{
	return v_accept_tps;
}

LONG C_NetServer::M_NetworkTPS()
{
	return v_network_tps;
}

LONG C_NetServer::M_NetworkAcceptCount()
{
	return v_network_session_count;
}