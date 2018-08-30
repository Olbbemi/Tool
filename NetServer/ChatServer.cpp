#include "Precompile.h"
#include "ChatServer.h"

#include "Serialize/Serialize.h"

#include <vector>

using namespace Olbbemi;

#define INDEX_VALUE 255

C_ChatServer::C_ChatServer()
{

}

unsigned int __stdcall C_ChatServer::M_UpdateThread(void* pa_argument)
{
	return ((C_ChatServer*)pa_argument)->M_UpdateProc();
}

unsigned int C_ChatServer::M_UpdateProc()
{



}

void C_ChatServer::VIR_OnClientJoin(LONGLONG pa_session_id, TCHAR* pa_ip, WORD pa_port)
{
	/*WORD lo_index = (WORD)(pa_session_id & INDEX_VALUE);
	ST_Session* new_session = new ST_Session;
	
	new_session->session_id = pa_session_id;

	m_session_list.insert(make_pair(lo_index, new_session));*/

	/*
		초기 플레이어 위치 구하여 패킷전송해주고 섹터에도 넘겨줌
	*/

	// DB 게임로그에 저장하는 것으로 처리
}

void C_ChatServer::VIR_OnClientLeave(LONGLONG pa_session_id)
{
	//WORD lo_index = (WORD)(pa_session_id & INDEX_VALUE);
	//int check = m_session_list.erase(lo_index);
	//if (check == 0)
	//{
	//	// error
	//}



	// DB 게임로그에 저장하는 것으로 처리
}

bool C_ChatServer::VIR_OnConnectionRequest(TCHAR* pa_ip, WORD pa_port)
{
	/*
	IP 및 Port 확인해서 접속하며안되는 영역은 차단해야함
	*/

	return true;
}

void C_ChatServer::VIR_OnRecv(LONGLONG pa_session_id, C_Serialize& pa_packet)
{
	char lo_buffer[4];
	C_Serialize *lo_serialQ = C_Serialize::S_Alloc();
	lo_serialQ->M_InputHeaderSize(5);

	memcpy_s(lo_buffer, 4, pa_packet.M_GetBufferPtr(), pa_packet.M_GetUsingSize());

	lo_serialQ->M_Enqueue(lo_buffer, 4);

	C_Serialize::S_AddReference(lo_serialQ);
	M_SendPacket(pa_session_id, lo_serialQ);

	C_Serialize::S_Free(lo_serialQ);
}

void C_ChatServer::VIR_OnError(int pa_line, TCHAR* pa_action, EN_LogState pa_log_level, ST_Log* pa_log)
{
	TCHAR lo_server[] = _TEXT("NONE");
	switch (pa_log_level)
	{
	case EN_LogState::system:	_LOG(pa_line, LOG_LEVEL_SYSTEM, pa_action, lo_server, pa_log->count, pa_log->log_str);
		break;

	case EN_LogState::error:	_LOG(pa_line, LOG_LEVEL_ERROR, pa_action, lo_server, pa_log->count, pa_log->log_str);	printf("\a");
		break;

	case EN_LogState::warning:	_LOG(pa_line, LOG_LEVEL_WARNING, pa_action, lo_server, pa_log->count, pa_log->log_str);
		break;

	case EN_LogState::debug:	_LOG(pa_line, LOG_LEVEL_DEBUG, pa_action, lo_server, pa_log->count, pa_log->log_str);
		break;
	}
}

void C_ChatServer::VIR_OnSend(LONGLONG pa_session_id, int pa_send_size) {}
void C_ChatServer::VIR_OnWorkerThreadBegin() {}
void C_ChatServer::VIR_OnWorkerThreadEnd() {}