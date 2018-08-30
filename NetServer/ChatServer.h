#ifndef ChatServer_Info
#define ChatServer_Info

#include "Log/Log.h"
#include "Serialize/Serialize.h"
#include "MemoryPool/MemoryPool.h"
#include "LockFreeQueue/LockFreeQueue.h"

#include "NetServer.h"

#include <unordered_map>
using namespace std;

namespace Olbbemi
{
	class C_ChatServer : public C_NetServer
	{
	private:
		struct ST_Session
		{
			LONGLONG session_id;



		};

		unordered_map<int, ST_Session*> m_session_list;
		//C_LFQueue<C_Serialize*> m_actor_queue;
		//C_MemoryPoolTLS<C_Serialize> m_tls_pool;

		unsigned int M_UpdateProc();
		static unsigned int __stdcall M_UpdateThread(void* pa_argument);

	public:
		C_ChatServer();
		~C_ChatServer();

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