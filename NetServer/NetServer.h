#ifndef NetServer_Info
#define NetServer_Info

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

	class C_NetServer
	{
	private:
		struct ST_Session
		{
			volatile LONG v_send_flag, v_session_info[2]; // [0]: is_alive, [1]: io_count
			volatile ULONG v_send_count;
			LONGLONG session_id;
			SOCKET socket;

			C_Serialize* store_buffer[500];

			C_RINGBUFFER *recvQ;
			C_LFQueue<C_Serialize*> *sendQ;

			OVERLAPPED *recvOver, *sendOver;
		};

#pragma pack(push, 1)
		struct ST_Header
		{
			BYTE code;
			WORD len;
			BYTE xor_code;
			BYTE checksum;
		};
#pragma pack(pop)

		bool m_is_nagle_on;
		TCHAR m_ip[17];
		BYTE m_workthread_count, m_packet_code, m_first_key, m_second_key;
		WORD m_port;
		DWORD m_max_session_count;
		LONGLONG m_session_count;

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
		void M_Release(LONGLONG pa_session_id);

		static unsigned int __stdcall M_AcceptThread(void* pa_argument);
		static unsigned int __stdcall M_WorkerThread(void* pa_argument);

	protected:
		enum class EN_LogState : BYTE
		{
			system = 0,
			error,
			warning,
			debug
		};

		void M_SendPacket(LONGLONG pa_session_id, C_Serialize* pa_packet);
		bool M_Disconnect(WORD pa_index);

		virtual void VIR_OnClientJoin(LONGLONG pa_session_id, TCHAR* pa_ip, WORD pa_port) = 0;
		virtual void VIR_OnClientLeave(LONGLONG pa_session_id) = 0;
		virtual bool VIR_OnConnectionRequest(TCHAR* pa_ip, WORD pa_port) = 0;

		virtual void VIR_OnRecv(LONGLONG pa_session_id, C_Serialize& pa_packet) = 0;
		virtual void VIR_OnSend(LONGLONG pa_session_id, int pa_send_size) = 0;

		virtual void VIR_OnWorkerThreadBegin() = 0;
		virtual void VIR_OnWorkerThreadEnd() = 0;

		virtual void VIR_OnError(int pa_line, TCHAR* pa_action, EN_LogState pa_log_level, ST_Log* pa_log) = 0;

	public:
		bool M_Start(bool pa_is_nagle_on, BYTE pa_work_count, TCHAR* pa_ip, WORD pa_port, DWORD pa_max_session, BYTE pa_packet_code, BYTE pa_first_key, BYTE pa_second_key);
		void M_Stop();

		void Encode(C_Serialize* pa_serialQ);
		bool Decode(C_Serialize* pa_serialQ);


		LONG M_TLSPoolAllocCount();
		LONG M_TLSPoolChunkCount();
		LONG M_TLSPoolNodeCount();

		LONG M_StackAllocCount();
		LONG M_StackUseCount();

		LONG M_QueueAllocCount();
		LONG M_QueueUseCount();
	};
}
#endif