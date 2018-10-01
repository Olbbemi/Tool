#ifndef CPU_Check_Info
#define CPU_Check_Info

#include <Windows.h>

namespace Olbbemi
{
	class C_CPU_Check
	{
	private:
		HANDLE m_process;
		int m_process_count;

		float m_process_total, m_process_user, m_process_kernel;
		float m_processor_total, m_processor_user, m_processor_kernel;

		ULARGE_INTEGER m_process_lastKernel, m_process_lastUser, m_process_lastTime;
		ULARGE_INTEGER m_processor_lastKernel, m_processor_lastUser, m_processor_lastIdle;

	public:
		C_CPU_Check(HANDLE pa_handle = INVALID_HANDLE_VALUE);

		void M_UpdateCPUTime();

		float M_ProcessTotal();
		float M_ProcessUser();
		float M_ProcessKernel();

		float M_ProcessorTotal();
		float M_ProcessorUser();
		float M_ProcessorKernel();
	};
}

#endif