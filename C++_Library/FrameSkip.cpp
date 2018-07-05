#include "Precompile.h"
#include "FrameSkip.h"

#include <timeapi.h>
#pragma comment(lib,"winmm.lib")

#define Sec 20

FRAMESKIP::FRAMESKIP()
{
	timeBeginPeriod(1);
	m_totaltime = 0;
	m_check = false;
}

FRAMESKIP::~FRAMESKIP()
{
	timeEndPeriod(1);
}

// Sec 값만 헤더에서 변경하면 됨 [프레임 = 1000 / sec]
bool FRAMESKIP::IsFrameSkip()
{
	m_is_flip = true;

	if (m_check == false)
	{
		m_pretime = timeGetTime();
		m_check = true;
	}
	else
	{
		m_curtime = timeGetTime();

		if (m_curtime - m_pretime >= Sec) 
			m_totaltime += (m_curtime - m_pretime) - Sec;
		else
			m_totaltime -= Sec - (m_curtime - m_pretime);

		if (m_totaltime >= Sec)
		{
			m_is_flip = false;
			m_pretime = m_curtime;
			m_totaltime -= Sec;
		}
		else if (m_totaltime <= 0)
		{
			Sleep(-m_totaltime);
			m_pretime = m_curtime + -m_totaltime;
			m_totaltime = 0;
		}
	}

	return m_is_flip;
}
