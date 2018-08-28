#ifndef Contents_Info
#define Contents_Info

#include "Log/Log.h"
#include "LanServer.h"

namespace Olbbemi
{
	class C_Contents : public C_LanServer
	{
	public:
		void VIR_OnClientJoin(LONGLONG pa_session_id, TCHAR* pa_ip, WORD pa_port) override;
		void VIR_OnClientLeave(LONGLONG pa_session_id) override;
		bool VIR_OnConnectionRequest(TCHAR* pa_ip, WORD pa_port) override;

		void VIR_OnRecv(LONGLONG pa_session_id, C_Serialize& pa_packet) override;
		void VIR_OnSend(LONGLONG pa_session_id, int pa_send_size) override;

		void VIR_OnWorkerThreadBegin() override;
		void VIR_OnWorkerThreadEnd() override;

		void VIR_OnError(int pa_line, TCHAR* pa_action, EN_LogState pa_log_level, ST_Log* pa_log) override;
	};
}

#endif