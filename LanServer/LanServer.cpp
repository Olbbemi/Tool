#include "Precompile.h"
#include "LanServer.h"

#include "Profile/Profile.h"
#include "Serialize/Serialize.h"
#include "RingBuffer/RingBuffer.h"

#include <process.h>
#include <strsafe.h>

using namespace Olbbemi;

/**------------------------------------------------------------------------
  * F1 Ű�� �������� ȣ��Ǵ� �Լ��� ������ �����ϱ����� ���ҽ��� ����
  * �迭�� �̿��Ͽ� ��� ���� ������ ���� ��� [ interlock �Լ��� ���� ]
  * m_session_list ����ü ���� session_info ���� [ 0: �ش� �迭�� ��뿩�θ� Ȯ��, 1: �ش� �迭�� IO_count ���� ]
  *------------------------------------------------------------------------*/
bool C_LanServer::M_Start(bool pa_is_nagle_on, BYTE pa_work_count, TCHAR* pa_ip, WORD pa_port, DWORD pa_max_session)
{
	TCHAR lo_action[] = _TEXT("LanServer");

	m_session_count = 0;
	m_is_nagle_on = pa_is_nagle_on;			m_workthread_count = pa_work_count;
	StringCchCopy(m_ip, 17, pa_ip);			m_port = pa_port;
	m_max_session_count = pa_max_session;

	m_probable_index = new C_LFStack<WORD>;

	m_thread_handle = new HANDLE[pa_work_count + 1];
	m_session_list = new ST_Session[pa_max_session];
	for (int i = 0; i < pa_max_session; i++)
	{
		m_session_list[i].v_session_info[1] = 0;
		m_session_list[i].v_session_info[0] = NO;
		m_probable_index->M_Push(i);
	}
		
	M_CreateIOCPHandle(m_iocp_handle);
	if (m_iocp_handle == NULL)
	{
		ST_Log *lo_log = new ST_Log({ "Create IOCP Handle Error Code: " + to_string(WSAGetLastError()) });
		VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);

		return false;
	}

	m_thread_handle[0] = (HANDLE)_beginthreadex(NULL, 0, M_AcceptThread, this, 0, nullptr);
	if (m_thread_handle[0] == 0)
	{
		ST_Log *lo_log = new ST_Log({ "Create AcceptThread Error Code: " + to_string(WSAGetLastError()) });
		VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);

		return false;
	}

	for (int i = 1; i < pa_work_count + 1; ++i)
	{
		m_thread_handle[i] = (HANDLE)_beginthreadex(NULL, 0, M_WorkerThread, this, 0, nullptr);
		if (m_thread_handle[i] == 0)
		{
			ST_Log *lo_log = new ST_Log({ "Create WorkerThread Error Code: " + to_string(WSAGetLastError()) });
			VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);

			return false;
		}
	}

	return true;
}

/**------------------------------------
  * F2 Ű�� �������� ȣ��Ǵ� �Լ��� ������ �����ϱ� ���� ���ҽ� ����
  * �������� �����ϹǷ� ����Ǿ� �ִ� ��� Ŭ���̾�Ʈ���� shutdown �� ���Ͽ� ���Ḧ �˸�
  * ��� Ŭ���̾�Ʈ�� ������� Ȯ���� �� �� ������ ���� �� ���ҽ� ����
  *------------------------------------*/
void C_LanServer::M_Stop()
{
	closesocket(m_listen_socket);
	for (int i = 0; i < m_max_session_count; ++i)
	{
		if(m_session_list[i].v_session_info[0] == YES)
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
	
	PostQueuedCompletionStatus(m_iocp_handle, 0, 0, 0);

	DWORD lo_wait_value = WaitForMultipleObjects(m_workthread_count + 1, m_thread_handle, TRUE, INFINITE);
	if (lo_wait_value == WAIT_FAILED)
	{
		TCHAR lo_action[] = _TEXT("LanServer");
		ST_Log *lo_log = new ST_Log({ "WaitForMulti Error Code: " + to_string(WSAGetLastError()) });
		VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
	}

	for (int i = 0; i < m_workthread_count + 1; ++i)
		CloseHandle(m_thread_handle[i]);

	delete[] m_thread_handle;	delete[] m_session_list;
	delete m_probable_index;

	_tprintf(_TEXT("Stop Success!!!\n"));
}

/**------------------------------------
  * IOCP �ڵ��� ��� ���� �Լ�
  *------------------------------------*/
void C_LanServer::M_CreateIOCPHandle(HANDLE& pa_handle)
{
	pa_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

/**------------------------------------
  * ���� ������ IOCP �� ����ϱ� ���� �Լ�
  *------------------------------------*/
bool C_LanServer::M_MatchIOCPHandle(SOCKET pa_socket, ST_Session* pa_session)
{
	HANDLE lo_handle_value = CreateIoCompletionPort((HANDLE)pa_socket, m_iocp_handle, (ULONG_PTR)pa_session, 2);
	if (lo_handle_value == NULL)
		return false;

	return true;
}

/**------------------------------------
  * Accept �� ������ ������
  *------------------------------------*/
unsigned int __stdcall C_LanServer::M_AcceptThread(void* pa_argument)
{
	return ((C_LanServer*)pa_argument)->M_Accept();
}

/**------------------------------------
  * ��Ŷ �ۼ����� ������ ������
  *------------------------------------*/
unsigned int __stdcall C_LanServer::M_WorkerThread(void* pa_argument)
{
	return ((C_LanServer*)pa_argument)->M_PacketProc();
}

/**------------------------------------
  * Accept �����忡�� ȣ���� �ݹ� �Լ�
  * ������ ���������� �̷������ �ش� Ŭ���̾�Ʈ�� ������ �迭�ε����� ������ ������ �̿��Ͽ� ����
  * �ش� �迭�� ���ҽ� �Ҵ� �� ������ IOCP�� ���
  * Stop �Լ����� listen_socket �� �����ϸ� accept ���� ���� �߻��Ͽ� �ش� �����带 �����ϴ� ��� �̿�
  *------------------------------------*/
unsigned int C_LanServer::M_Accept()
{
	TCHAR lo_action[] = _TEXT("LanServer");

	WSADATA lo_wsadata;
	SOCKET lo_client_socket;
	SOCKADDR_IN lo_server_address, lo_client_address;
	int lo_error_check, lo_len = sizeof(lo_server_address);

	WSAStartup(MAKEWORD(2, 2), &lo_wsadata);

	m_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listen_socket == INVALID_SOCKET)
	{
		ST_Log *lo_log = new ST_Log({ "Listen Socket Error Code: " + to_string(WSAGetLastError()) });
		VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
		
		return 0;
	}

	ZeroMemory(&lo_server_address, sizeof(lo_server_address));
	lo_server_address.sin_family = AF_INET;
	WSAStringToAddress(m_ip, AF_INET, NULL, (SOCKADDR*)&lo_server_address, &lo_len);
	WSAHtons(m_listen_socket, m_port, &lo_server_address.sin_port);

	lo_error_check = bind(m_listen_socket, (SOCKADDR*)&lo_server_address, sizeof(lo_server_address));
	if (lo_error_check == SOCKET_ERROR)
	{
		ST_Log *lo_log = new ST_Log({ "Socket Bind Error Code: " + to_string(WSAGetLastError()) });
		VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
		
		return 0;
	}

	lo_error_check = listen(m_listen_socket, SOMAXCONN);
	if (lo_error_check == SOCKET_ERROR)
	{
		ST_Log *lo_log = new ST_Log({ "Listen Error Code: " + to_string(WSAGetLastError()) });
		VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
		
		return 0;
	}

	while (1)
	{
		lo_client_socket = accept(m_listen_socket, (SOCKADDR*)&lo_client_address, &lo_len);
		if (lo_client_socket == INVALID_SOCKET)
		{
			int lo_error = WSAGetLastError();
			if (lo_error == WSAEINTR || lo_error == WSAENOTSOCK)
				break;

			ST_Log *lo_log = new ST_Log({ "Socket Accept Error Code: " + to_string(lo_error) });
			VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
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

			ST_Log *lo_log = new ST_Log({ "Ip: " + lo_via_buffer + ", Port: " + to_string(lo_confirm_port) + "Can't Accpet" });
			VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);			
			closesocket(lo_client_socket);

			continue;
		}

		if (m_probable_index->M_GetUseCount() == 0)
		{
			closesocket(lo_client_socket);
			continue;
		}
		
		lo_avail_index = m_probable_index->M_Pop();
		m_session_list[lo_avail_index].socket = lo_client_socket;	m_session_list[lo_avail_index].v_send_count = 0;
		m_session_list[lo_avail_index].recvOver = new OVERLAPPED;	m_session_list[lo_avail_index].sendOver = new OVERLAPPED;
		m_session_list[lo_avail_index].recvQ = new C_RINGBUFFER;	m_session_list[lo_avail_index].sendQ = new C_LFQueue<C_Serialize*>;
		m_session_list[lo_avail_index].v_session_info[1] = 0;		m_session_list[lo_avail_index].session_id = ((m_session_count << 16) | lo_avail_index);
		m_session_list[lo_avail_index].v_session_info[0] = YES;		m_session_list[lo_avail_index].v_send_flag = FALSE;
		m_session_count++;

		lo_iocp_check = M_MatchIOCPHandle(lo_client_socket, &m_session_list[lo_avail_index]);
		if (lo_iocp_check == false)
		{
			ST_Log *lo_log = new ST_Log({ "IOCP Matching Error Code: " + to_string(WSAGetLastError()) });
			VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
			M_Release(m_session_list[lo_avail_index].session_id);
			
			continue;
		}

		InterlockedIncrement(&m_session_list[lo_avail_index].v_session_info[1]);

		VIR_OnClientJoin(m_session_list[lo_avail_index].session_id, lo_confirm_ip, lo_confirm_port);
		M_RecvPost(&m_session_list[lo_avail_index]);


		LONG interlock_value = InterlockedDecrement(&m_session_list[lo_avail_index].v_session_info[1]);
		if (interlock_value == 0)
			M_Release(m_session_list[lo_avail_index].session_id);
	}

	WSACleanup();
	return 1;
}

/**------------------------------------
  * Worker �����忡�� ȣ���� �ݹ� �Լ�
  * IOCP �� �̿��Ͽ� ��� Ŭ���̾�Ʈ�� ��Ŷ �ۼ����� ó��
  * �� ���Ǹ��� IO_count �� �����ϰ� ������ �� ���� 0�� �Ǵ� ��쿡�� ���� ����
  * �ܺο��� PostQueuedCompletionStatus �Լ��� ȣ���ϸ� �ش� �����尡 ����Ǵ� ����
  *------------------------------------*/
unsigned int C_LanServer::M_PacketProc()
{
	TCHAR lo_profile_name[] = _TEXT("worker");
	TCHAR lo_action[] = _TEXT("LanServer");

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
			PostQueuedCompletionStatus(m_iocp_handle, 0, 0, 0);
			break;
		}
		else if (lo_overlap == nullptr)
		{
			ST_Log *lo_log = new ST_Log({ "GQCS Error Code: " + to_string(WSAGetLastError()) });
			VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
		}
		else if (lo_transfered != 0 && lo_overlap == lo_session->recvOver)
		{	
			lo_session->recvQ->M_MoveRear(lo_transfered);

			while (1)
			{
				WORD lo_length;
				int lo_size = 0, lo_recv_size;
				C_Serialize lo_serialQ;
				lo_serialQ.M_InputHeaderSize(2);

				lo_recv_size = lo_session->recvQ->M_GetUseSize();

				if (lo_recv_size < 2)
					break;
					
				lo_session->recvQ->M_Peek((char*)&lo_length, 2, lo_size);
				if (lo_recv_size < lo_size + 2)
					break;
					
				lo_session->recvQ->M_Dequeue((char*)&lo_length, 2);
				lo_session->recvQ->M_Dequeue(lo_serialQ.M_GetBufferPtr(), lo_length);
				lo_serialQ.M_MoveRear(lo_length);

				VIR_OnRecv(lo_session->session_id, lo_serialQ);
			}

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
				// �� �� ���� �����͸� ���⵵�� ����
				ST_Log *lo_log = new ST_Log({ "IO Count is Nagative: " + to_string(interlock_value) });
				VIR_OnError(__LINE__, lo_action, EN_LogState::system, lo_log);
			}
		}
	}

	return 1;
}

/**------------------------------------ 
  * 
  *------------------------------------*/
void C_LanServer::M_SendPacket(LONGLONG pa_session_id, C_Serialize* pa_packet)
{
	WORD lo_index = (WORD)(pa_session_id & INDEX_VALUE);
	LONGLONG lo_id = (pa_session_id >> 16);

	if (m_session_list[lo_index].session_id != pa_session_id)
	{
		C_Serialize::S_Free(pa_packet);
		return;
	}
	
	// �̺κ��� �Լ��� �����ϱ�
	InterlockedIncrement(&m_session_list[lo_index].v_session_info[1]);
	if (InterlockedCompareExchange(&m_session_list[lo_index].v_session_info[0], NO, NO) == NO)
	{
	
		return;
	}
	
	C_Serialize::S_AddReference(pa_packet);

	m_session_list[lo_index].sendQ->M_Enqueue(pa_packet);
	M_SendPost(&m_session_list[lo_index]);

	C_Serialize::S_Free(pa_packet);
	LONG lo_count = InterlockedDecrement(&m_session_list[lo_index].v_session_info[1]);
	if (lo_count == 0)
		M_Release(pa_session_id);
}

// ������ ������ �Ű����� ���� �� ���� �����ϱ�
bool C_LanServer::M_Disconnect(WORD pa_index)
{
	/*
	
	
	*/

	//if (m_session_list[pa_index].v_is_alive == false)
	//{
	//	// error
	//	return false;
	//}

	shutdown(m_session_list[pa_index].socket, SD_BOTH);
	return true;
}

/**------------------------------------
  * �� ���ǿ� ���� ������ ó���ϱ� ���� �Լ�
  *------------------------------------*/
bool C_LanServer::M_RecvPost(ST_Session* pa_session)
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

/**------------------------------------
  * �� ���ǿ� ���� �۽��� ó���ϱ� ���� �Լ�
  *------------------------------------*/
char C_LanServer::M_SendPost(ST_Session* pa_session)
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

/**------------------------------------
  * ������ �����ϱ� ���� ȣ��Ǵ� �Լ� [ �ش� ������ ����� ��� ���ҽ� ���� �� ��ȯ ]
  *------------------------------------*/
void C_LanServer::M_Release(LONGLONG pa_session_id)
{
	WORD lo_index = (WORD)(pa_session_id & INDEX_VALUE);

	LONG lo_session_info[2];
	lo_session_info[0] = m_session_list[lo_index].v_session_info[0];
	lo_session_info[1] = m_session_list[lo_index].v_session_info[1];

	if (InterlockedCompareExchange64((LONG64*)m_session_list[lo_index].v_session_info, NO, *((LONG64*)lo_session_info)) != *((LONG64*)lo_session_info))
		return;

	VIR_OnClientLeave(pa_session_id);

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
}

//============================================================================================================//

/**---------------------------------
  * �������� ���������� ����ϴ� �Լ�
  *---------------------------------*/

LONG C_LanServer::M_TLSPoolAllocCount()
{
	return C_Serialize::S_TLSAllocCount();
}

LONG C_LanServer::M_TLSPoolChunkCount()
{
	return C_Serialize::S_TLSChunkCount();
}

LONG C_LanServer::M_TLSPoolNodeCount()
{
	return C_Serialize::S_TLSNodeCount();
}

LONG C_LanServer::M_StackAllocCount()
{
	return m_probable_index->M_GetAllocCount();
}

LONG C_LanServer::M_StackUseCount()
{
	return m_probable_index->M_GetUseCount();
}

LONG C_LanServer::M_QueueAllocCount()
{
	LONG return_value = 0;
	for (int i = 0; i < m_max_session_count; i++)
	{
		if (m_session_list[i].v_session_info[0] == YES)
			return_value += m_session_list[i].sendQ->M_GetAllocCount();
	}

	return return_value;
}

LONG C_LanServer::M_QueueUseCount()
{
	LONG return_value = 0;
	for (int i = 0; i < m_max_session_count; i++)
	{
		if (m_session_list[i].v_session_info[0] == YES)
			return_value += m_session_list[i].sendQ->M_GetUseCount();
	}

	return return_value;
}