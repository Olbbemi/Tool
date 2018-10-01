#include "CPU_Check.h"

using namespace Olbbemi;

/**-------------------------------------------------
  * CPU 계산을 위한 데이터 초기화 및 프로세스 핸들 얻기
  *-------------------------------------------------*/
C_CPU_Check::C_CPU_Check(HANDLE pa_handle)
{
	SYSTEM_INFO lo_system;

	if (pa_handle == INVALID_HANDLE_VALUE)
		m_process = GetCurrentProcess();
	
	GetSystemInfo(&lo_system);
	m_process_count = lo_system.dwNumberOfProcessors;

	m_process_total = 0;
	m_process_user = 0;
	m_process_kernel = 0;
	
	m_processor_total = 0;
	m_processor_user = 0;
	m_processor_kernel = 0;

	m_process_lastUser.QuadPart = 0;
	m_process_lastKernel.QuadPart = 0;
	m_process_lastTime.QuadPart = 0;

	m_processor_lastUser.QuadPart = 0;
	m_processor_lastKernel.QuadPart = 0;
	m_processor_lastIdle.QuadPart = 0;

	M_UpdateCPUTime();
}

/**-----------------------------------------------------------------
  * 하드웨어 전체 및 특정 프로세스의 커널 및 유저 사용량을 계산하는 함수
  *-----------------------------------------------------------------*/
void C_CPU_Check::M_UpdateCPUTime()
{
	ULONG64 lo_processor_user_gap, lo_processor_kernel_gap, lo_processor_idle_gap, lo_processor_total;
	ULONG64 lo_process_user_gap, lo_process_kernel_gap, lo_process_time_gap, lo_process_total;
	ULARGE_INTEGER lo_processor_user, lo_processor_kernel, lo_processor_idle;
	ULARGE_INTEGER lo_process_user, lo_process_kernel;
	ULARGE_INTEGER lo_none, lo_process_time;

	if (GetSystemTimes((PFILETIME)&lo_processor_idle, (PFILETIME)&lo_processor_kernel, (PFILETIME)&lo_processor_user) == false)
		return;
	
	/**---------------------------------------------
	 * 해당 하드웨어 전체의 커널 및 유저 사용량을 계산
	 *----------------------------------------------*/
	lo_processor_user_gap = lo_processor_user.QuadPart - m_processor_lastUser.QuadPart;
	lo_processor_kernel_gap = lo_processor_kernel.QuadPart - m_processor_lastKernel.QuadPart;
	lo_processor_idle_gap = lo_processor_idle.QuadPart - m_processor_lastIdle.QuadPart;

	lo_processor_total = lo_processor_kernel_gap + lo_processor_user_gap;
	if (lo_processor_total == 0)
	{
		m_processor_total = 0;
		m_processor_user = 0;
		m_processor_kernel = 0;
	}
	else
	{
		if (lo_processor_total - lo_processor_idle_gap < 0)
			m_processor_total = 0;
		else
			m_processor_total = (float)((double)(lo_processor_total - lo_processor_idle_gap) / lo_processor_total * 100.0f);
		
		if (lo_processor_kernel_gap - lo_processor_idle_gap < 0)
			m_processor_kernel = 0;
		else
			m_processor_kernel = (float)((double)(lo_processor_kernel_gap - lo_processor_idle_gap) / lo_processor_total * 100.0f);
	
		m_processor_user = (float)((double)(lo_processor_user_gap) / lo_processor_total * 100.0f);
	}

	m_processor_lastUser = lo_processor_user;
	m_processor_lastKernel = lo_processor_kernel;
	m_processor_lastIdle = lo_processor_idle;

	/**----------------------------------------
	 * 해당 프로세스의 커널 및 유저 사용량을 계산
	 *-----------------------------------------*/
	GetSystemTimeAsFileTime((LPFILETIME)&lo_process_time);
	GetProcessTimes(m_process, (LPFILETIME)&lo_none, (LPFILETIME)&lo_none, (LPFILETIME)&lo_process_kernel, (LPFILETIME)&lo_process_user);

	lo_process_time_gap = lo_process_time.QuadPart - m_process_lastTime.QuadPart;
	lo_process_user_gap = lo_process_user.QuadPart - m_process_lastUser.QuadPart;
	lo_process_kernel_gap = lo_process_kernel.QuadPart - m_process_lastKernel.QuadPart;

	lo_process_total = lo_process_kernel_gap + lo_process_user_gap;

	m_process_total = (float)(lo_process_total / (double)m_process_count / (double)lo_process_time_gap * 100.0f);
	m_process_kernel = (float)(lo_process_kernel_gap / (double)m_process_count / (double)lo_process_time_gap * 100.0f);
	m_process_user = (float)(lo_process_user_gap / (double)m_process_count / (double)lo_process_time_gap * 100.0f);

	m_process_lastTime = lo_process_time;
	m_process_lastKernel = lo_process_kernel;
	m_process_lastUser = lo_process_user;
}

float C_CPU_Check::M_ProcessTotal()
{
	return m_process_total;
}
float C_CPU_Check::M_ProcessUser()
{
	return m_process_user;
}

float C_CPU_Check::M_ProcessKernel()
{
	return m_process_kernel;
}

float C_CPU_Check::M_ProcessorTotal()
{
	return m_processor_total;
}

float C_CPU_Check::M_ProcessorUser()
{
	return m_processor_user;
}

float C_CPU_Check::M_ProcessorKernel()
{
	return m_processor_kernel;
}