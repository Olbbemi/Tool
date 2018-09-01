#include "Precompile.h"
#include "Contents.h"

#include "Serialize/Serialize.h"
using namespace Olbbemi;

void C_Contents::VIR_OnClientJoin(LONGLONG pa_session_id, TCHAR* pa_ip, WORD pa_port)
{
	WORD lo_header;
	C_Serialize *lo_serialQ = C_Serialize::S_Alloc(2);
	//lo_serialQ->M_InputHeaderSize(2);

	__int64 lo_data = 0x7fffffffffffffff;

	lo_serialQ->M_Enqueue((char*)&lo_data, 8);

	C_Serialize::S_AddReference(lo_serialQ);
	M_SendPacket(pa_session_id, lo_serialQ);

	C_Serialize::S_Free(lo_serialQ);

	// DB 게임로그에 저장하는 것으로 처리
}

void C_Contents::VIR_OnClientLeave(LONGLONG pa_session_id)
{
	// DB 게임로그에 저장하는 것으로 처리
}

bool C_Contents::VIR_OnConnectionRequest(TCHAR* pa_ip, WORD pa_port)
{
	/*
		IP 및 Port 확인해서 접속하며안되는 영역은 차단해야함
	*/

	return true;
}

void C_Contents::VIR_OnRecv(LONGLONG pa_session_id, C_Serialize& pa_packet)
{
	char lo_buffer[8];
	
	C_Serialize *lo_serialQ = C_Serialize::S_Alloc(2);
	//lo_serialQ->M_InputHeaderSize(2);

	memcpy_s(lo_buffer, 8, pa_packet.M_GetBufferPtr(), pa_packet.M_GetUsingSize());
	lo_serialQ->M_Enqueue(lo_buffer, 8);

	C_Serialize::S_AddReference(lo_serialQ);
	M_SendPacket(pa_session_id, lo_serialQ);

	C_Serialize::S_Free(lo_serialQ);
}

void C_Contents::VIR_OnError(int pa_line, TCHAR* pa_action, EN_LogState pa_log_level, ST_Log* pa_log)
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

void C_Contents::VIR_OnSend(LONGLONG pa_session_id, int pa_send_size) {}
void C_Contents::VIR_OnWorkerThreadBegin() {}
void C_Contents::VIR_OnWorkerThreadEnd() {}