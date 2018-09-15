#ifndef LanServer_Info
#define LanServer_Info

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32")

#include "LockFreeStack/LockFreeStack.h"
#include "LockFreeQueue/LockFreeQueue.h"

namespace Olbbemi
{
	class C_Serialize;
	class C_RINGBUFFER;

	class C_LanServer
	{
	private:
		struct ST_Session
		{
			SOCKET socket;
			WORD alloc_index;

			volatile LONG v_send_flag, io_count;
			volatile ULONG v_send_count;
			
			C_Serialize* store_buffer[500];

			C_RINGBUFFER *recvQ;
			C_LFQueue<C_Serialize*> *sendQ;

			OVERLAPPED recvOver, sendOver;
		};

		bool m_is_nagle_on;
		TCHAR m_ip[17], m_log_action[20];
		BYTE m_make_work_count, m_run_work_count;
		WORD m_port;
		DWORD m_max_client_count;

		SOCKET m_listen_socket;
		HANDLE m_iocp_handle, *m_thread_handle;
		ST_Session* m_session_list;

		C_LFStack<WORD> *m_probable_index;

		void M_CreateIOCPHandle(HANDLE& pa_handle);
		bool M_MatchIOCPHandle(SOCKET pa_socket, ST_Session* pa_session);

		unsigned int M_Accept();
		unsigned int M_PacketProc();

		bool M_RecvPost(ST_Session* pa_session);
		char M_SendPost(ST_Session* pa_session);
		void M_Release(WORD pa_index);

		static unsigned int __stdcall M_AcceptThread(void* pa_argument);
		static unsigned int __stdcall M_WorkerThread(void* pa_argument);

	protected:
		enum class E_LogState : BYTE
		{
			system = 0,
			error,
			warning,
			debug
		};

		void M_SendPacket(WORD pa_index, C_Serialize* pa_packet);

	public:	
		void M_LanStart(bool pa_is_nagle_on, BYTE pa_run_work_count, BYTE pa_make_work_count, TCHAR* pa_ip, WORD pa_port, BYTE pa_max_client);
		void M_LanStop();

		void M_AnnounceNewClient();


		virtual void VIR_OnClose() = 0;

		virtual void VIR_OnClientJoin(BYTE pa_index, TCHAR* pa_ip, WORD pa_port) = 0;
		virtual void VIR_OnClientLeave(BYTE pa_index) = 0;
		
		virtual void VIR_OnRecv(BYTE pa_index, C_Serialize* pa_packet) = 0;
		virtual void VIR_OnSend(BYTE pa_index, int pa_send_size) = 0;

		virtual void VIR_OnError(int pa_line, TCHAR* pa_action, E_LogState pa_log_level, ST_Log* pa_log) = 0;
	};
}
#endif