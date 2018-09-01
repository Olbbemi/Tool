#include "Precompile.h"
#include "ChatServer.h"

#include "Protocol/PacketProtocol.h"

#include <vector>

using namespace Olbbemi;

#define HEAD_SIZE 2
#define INDEX_VALUE 255

C_ChatServer::C_ChatServer()
{
	m_iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocp_handle == NULL)
	{
		//Error
	}

	m_sector = new C_Sector;
	m_message_pool = new C_MemoryPool<ST_MESSAGE>(1000, false);
}

unsigned int __stdcall C_ChatServer::M_UpdateThread(void* pa_argument)
{
	return ((C_ChatServer*)pa_argument)->M_UpdateProc();
}

unsigned int C_ChatServer::M_UpdateProc()
{
	while (1)
	{
		DWORD lo_transferred;
		void* lo_completion_key;
		OVERLAPPED *lo_over;

		GetQueuedCompletionStatus(m_iocp_handle, &lo_transferred, (ULONG_PTR*)&lo_completion_key, &lo_over, INFINITE);

		if (lo_transferred == FALSE)
			break;

		if (lo_transferred == TRUE)
		{
			while (m_actor_queue.M_GetUseCount() != 0)
			{
				ST_Player* new_player;
				ST_MESSAGE* lo_message;

				m_actor_queue.M_Dequeue(lo_message);
				switch (lo_message->type)
				{
					case E_MSG_TYPE::join:
						new_player = new ST_Player;
						
						new_player->account_no;
						new_player->id;
						new_player->nickname;
						new_player->session_id = lo_message->sessionID;
						new_player->xpos;
						new_player->ypos;

						//m_sector->SetUnitSectorPosition();

						m_session_list.insert(make_pair(lo_message->sessionID, new_player));
						break;

					case E_MSG_TYPE::contents:
						switch (*((WORD*)lo_message->payload))
						{
							case en_PACKET_CS_CHAT_REQ_LOGIN:		M_Login((ST_RES_LOGIN*)((char*)lo_message->payload + HEAD_SIZE));				break;
							case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:	M_MoveSector((ST_REQ_MOVE_SECTOR*)((char*)lo_message->payload + HEAD_SIZE));	break;
							case en_PACKET_CS_CHAT_REQ_MESSAGE:		M_Chatting((ST_REQ_CHAT*)((char*)lo_message->payload + HEAD_SIZE));				break;
						}

						break;

					case E_MSG_TYPE::leave:
						LONG64 lo_check = m_session_list.erase(lo_message->sessionID);
						if (lo_check == 0)
						{
							//error
						}

						break;
				}

				m_message_pool->M_Free(lo_message);
			}




		}
		else
		{
			//Error
		}




	}

	return 0;
}

void C_ChatServer::M_Login(ST_RES_LOGIN* pa_payload)
{

}

void C_ChatServer::M_MoveSector(ST_REQ_MOVE_SECTOR* pa_payload)
{

}
void C_ChatServer::M_Chatting(ST_REQ_CHAT* pa_payload)
{

}





void C_ChatServer::VIR_OnClientJoin(LONG64 pa_session_id, TCHAR* pa_ip, WORD pa_port)
{
	ST_MESSAGE* lo_message = m_message_pool->M_Alloc();

	lo_message->type = E_MSG_TYPE::join;
	lo_message->sessionID = pa_session_id;
	lo_message->payload = nullptr;

	m_actor_queue.M_Enqueue(lo_message);

	// DB 게임로그에 저장하는 것으로 처리
}

void C_ChatServer::VIR_OnClientLeave(LONG64 pa_session_id)
{
	ST_MESSAGE* lo_message = m_message_pool->M_Alloc();

	lo_message->type = E_MSG_TYPE::leave;
	lo_message->sessionID = pa_session_id;
	lo_message->payload = nullptr;

	m_actor_queue.M_Enqueue(lo_message);

	// DB 게임로그에 저장하는 것으로 처리
}

bool C_ChatServer::VIR_OnConnectionRequest(TCHAR* pa_ip, WORD pa_port)
{
	/*
		IP 및 Port 확인해서 접속하며안되는 영역은 차단해야함
	*/

	return true;
}

void C_ChatServer::VIR_OnRecv(LONG64 pa_session_id, C_Serialize& pa_packet)
{
	ST_MESSAGE* lo_message = m_message_pool->M_Alloc();
	C_Serialize* lo_serialQ = C_Serialize::S_Alloc(HEAD_SIZE);

	char* packet_ptr = pa_packet.M_GetBufferPtr();
	WORD lo_head = *((WORD*)packet_ptr);

	lo_serialQ->M_Enqueue(packet_ptr + HEAD_SIZE, pa_packet.M_GetUsingSize() - HEAD_SIZE);
	lo_serialQ->M_MakeHeader((char*)&lo_head, HEAD_SIZE);

	lo_message->type = E_MSG_TYPE::contents;
	lo_message->sessionID = pa_session_id;
	lo_message->payload = lo_serialQ;

	C_Serialize::S_AddReference(lo_serialQ);
	m_actor_queue.M_Enqueue(lo_message);

	C_Serialize::S_Free(lo_serialQ);
}

void C_ChatServer::VIR_OnError(int pa_line, TCHAR* pa_action, E_LogState pa_log_level, ST_Log* pa_log)
{
	TCHAR lo_server[] = _TEXT("NONE");
	switch (pa_log_level)
	{
		case E_LogState::system:	_LOG(pa_line, LOG_LEVEL_SYSTEM, pa_action, lo_server, pa_log->count, pa_log->log_str);
			break;

		case E_LogState::error:		_LOG(pa_line, LOG_LEVEL_ERROR, pa_action, lo_server, pa_log->count, pa_log->log_str);	printf("\a");
			break;

		case E_LogState::warning:	_LOG(pa_line, LOG_LEVEL_WARNING, pa_action, lo_server, pa_log->count, pa_log->log_str);
			break;

		case E_LogState::debug:		_LOG(pa_line, LOG_LEVEL_DEBUG, pa_action, lo_server, pa_log->count, pa_log->log_str);
			break;
	}
}

void C_ChatServer::VIR_OnSend(LONG64 pa_session_id, int pa_send_size) {}
void C_ChatServer::VIR_OnWorkerThreadBegin() {}
void C_ChatServer::VIR_OnWorkerThreadEnd() {}