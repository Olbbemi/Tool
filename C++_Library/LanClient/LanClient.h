#ifndef LanClient_Info
#define LanClient_Info

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32")

#include "Log/Log.h"
#include "LockFreeStack/LockFreeStack.h"
#include "LockFreeQueue/LockFreeQueue.h"

namespace Olbbemi
{
	class C_Serialize;
	class C_RINGBUFFER;

	class C_LanClient
	{
	private:
		struct ST_Session
		{
			SOCKET socket;
			volatile LONG v_send_flag, io_count;
			volatile ULONG v_send_count;

			C_Serialize* store_buffer[500];

			C_RINGBUFFER *recvQ;
			C_LFQueue<C_Serialize*> *sendQ;

			OVERLAPPED recvOver, sendOver;
		};

		BYTE m_make_work_count, m_run_work_count;

		
		HANDLE m_iocp_handle, *m_thread_handle;
		ST_Session m_session;


		void M_CreateIOCPHandle(HANDLE& pa_handle);
		bool M_MatchIOCPHandle(SOCKET pa_socket, ST_Session* pa_session);

		unsigned int M_PacketProc();

		bool M_RecvPost(ST_Session* pa_session);
		char M_SendPost(ST_Session* pa_session);
		void M_Release();

		static unsigned int __stdcall M_WorkerThread(void* pa_argument);

	protected:
		enum class E_LogState : BYTE
		{
			error = 0,
			warning,
			debug
		};

		void M_SendPacket(C_Serialize* pa_packet);

	public:
		volatile LONG v_network_tps;

		C_LanClient(bool pa_is_nagle_on, BYTE pa_run_work_count, BYTE pa_make_work_count, TCHAR* pa_ip, WORD pa_port);
		~C_LanClient();

		LONG M_TLSPoolAllocCount();
		LONG M_TLSPoolChunkCount();
		LONG M_TLSPoolNodeCount();

		LONG M_StackAllocCount();
		LONG M_StackUseCount();

		LONG M_AcceptTPS();
		LONG M_NetworkTPS();
		LONG M_NetworkAcceptCount();
		LONG64 M_TotalAcceptCount();
	};
}
#endif