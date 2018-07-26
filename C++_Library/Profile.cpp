#include "Precompile.h"
#include "Profile.h"

#include <time.h>
#include <stdlib.h>
#include <strsafe.h>

using namespace Olbbemi;

PROFILE::NODE::NODE(const PTCHAR p_str, _int64 p_time)
{
	m_min_count = m_max_count = 0;
	m_call_count = m_total_time = 0;
	m_start_time = p_time;
	StringCchCopy(m_function_name, _countof(m_function_name), p_str);

	for (int i = 0; i < 3; i++)
	{
		m_max_value[i] = 0;
		m_min_value[i] = 0x7FFFFFFFFFFFFFFF;
	}
}

PROFILE::PROFILE() : m_is_lock(true), m_file(_TEXT("Profile_"))
{
	QueryPerformanceFrequency(&m_frequency);
}

void PROFILE::GetTime()
{
	TCHAR date[20], tail[5] = _TEXT(".txt");
	time_t timer = time(NULL);

	struct tm data;
	localtime_s(&data, &timer);

	StringCchPrintf(date, 20, _TEXT("%4d_%02d%02d_%02d%02d%02d"), data.tm_year + 1900, data.tm_mon + 1, data.tm_mday, data.tm_hour, data.tm_min, data.tm_sec);
	StringCchCat(m_file, _countof(m_file), date);
	StringCchCat(m_file, _countof(m_file), tail);
}

void Olbbemi::Profile_Begin(PROFILE *p_obj, const PTCHAR p_str, int p_line)
{
	int flag = (unsigned int)PROFILE::STATUS::e_Normal;
	for (auto p = p_obj->node_list.begin(); p != p_obj->node_list.end(); p++)
	{
		if (!_tcscmp((*p)->m_function_name, p_str))
		{
			if ((*p)->m_start_time != 0)
				flag = (unsigned int)PROFILE::STATUS::e_Fail;
			else
			{
				flag = (unsigned int)PROFILE::STATUS::e_Success;
				QueryPerformanceCounter(&p_obj->begin_count);
				(*p)->m_start_time = p_obj->begin_count.QuadPart;
			}

			break;
		}
	}

	if (flag == (unsigned int)PROFILE::STATUS::e_Fail)
	{
		_tprintf(_TEXT("%d : Need to End Function"), p_line);
		throw;
	}
	else if (flag == (unsigned int)PROFILE::STATUS::e_Normal)
	{
		QueryPerformanceCounter(&p_obj->begin_count);
		PROFILE::NODE *temp = new PROFILE::NODE(p_str, p_obj->begin_count.QuadPart);
		p_obj->node_list.push_back(temp);
	}
}

void Olbbemi::Profile_End(PROFILE *p_obj, const PTCHAR p_str)
{
	QueryPerformanceCounter(&p_obj->end_count);
	for (auto p = p_obj->node_list.begin(); p != p_obj->node_list.end(); p++)
	{
		if (!_tcscmp((*p)->m_function_name, p_str))
		{
			(*p)->m_call_count++;
			(*p)->m_total_time += p_obj->end_count.QuadPart - (*p)->m_start_time;

			if ((*p)->m_min_count != 3 && (*p)->m_min_value[(*p)->m_min_count] > p_obj->end_count.QuadPart - (*p)->m_start_time)
				(*p)->m_min_value[(*p)->m_min_count++] = p_obj->end_count.QuadPart - (*p)->m_start_time;
			else if ((*p)->m_min_count == 3)
			{
				int index = 0;
				_int64 val = (*p)->m_min_value[0];
				for (int i = 1; i < 3; i++)
				{
					if (val < (*p)->m_min_value[i])
					{
						val = (*p)->m_min_value[i];
						index = i;
					}
				}

				if (p_obj->end_count.QuadPart - (*p)->m_start_time < val)
					(*p)->m_min_value[index] = p_obj->end_count.QuadPart - (*p)->m_start_time;
			}

			if ((*p)->m_max_count != 3 && (*p)->m_max_value[(*p)->m_max_count] < p_obj->end_count.QuadPart - (*p)->m_start_time)
				(*p)->m_max_value[(*p)->m_max_count++] = p_obj->end_count.QuadPart - (*p)->m_start_time;
			else if ((*p)->m_max_count == 3)
			{
				int index = 0;
				_int64 val = (*p)->m_max_value[0];
				for (int i = 1; i < 3; i++)
				{
					if (val >(*p)->m_max_value[i])
					{
						val = (*p)->m_max_value[i];
						index = i;
					}
				}

				if (p_obj->end_count.QuadPart - (*p)->m_start_time > val)
					(*p)->m_max_value[index] = p_obj->end_count.QuadPart - (*p)->m_start_time;
			}

			(*p)->m_start_time = 0;
			break;
		}
	}
}

void PROFILE::Save()
{
	FILE *output;

	if (GetAsyncKeyState('L') & 0x0001)
	{
		m_is_lock = true;
		_tprintf(_TEXT("-----L O C K-----\n"));
	}

	if (GetAsyncKeyState('U') & 0x0001)
	{
		m_is_lock = false;
		_tprintf(_TEXT("-----U N L O C K-----\n"));
	}

	if (m_is_lock == false && (GetAsyncKeyState(VK_HOME) & 0x0001))
	{
		PROFILE::GetTime();

		_tfopen_s(&output, m_file, _TEXT("w, ccs=UTF-16LE"));
		_ftprintf_s(output, _TEXT("  -----------------------------------------------------------------------------------------------------------------------\n"));
		_ftprintf_s(output, _TEXT("#            Name            #       Average       #       Min_Value       #       Max_Value       #       Count       #\n"));

		for (auto p = node_list.begin(); p != node_list.end(); p++)
		{
			_int64 MaxValue = (*p)->m_max_value[0], MinValue = (*p)->m_min_value[0];

			for (int i = 1; i < (*p)->m_max_count; i++)
			{
				if ((*p)->m_max_value[i] > MaxValue)
				{
					(*p)->m_total_time -= MaxValue;
					MaxValue = (*p)->m_max_value[i];
				}
				else
					(*p)->m_total_time -= (*p)->m_max_value[i];
			}
			(*p)->m_call_count -= ((*p)->m_max_count - 1);

			for (int i = 1; i < (*p)->m_min_count; i++)
			{
				if ((*p)->m_min_value[i] < MinValue)
				{
					(*p)->m_total_time -= MinValue;
					MinValue = (*p)->m_min_value[i];
				}
				else
					(*p)->m_total_time -= (*p)->m_min_value[i];
			}
			(*p)->m_call_count -= ((*p)->m_min_count - 1);

			_ftprintf_s(output, _TEXT("#%30s#%22.4Lf#%24.4Lf#%24.4Lf#%18lld#\n"), (*p)->m_function_name, ((long double)(*p)->m_total_time / (long double)(*p)->m_call_count / (long double)m_frequency.QuadPart * MICRO), ((long double)MinValue / (long double)m_frequency.QuadPart * MICRO), ((long double)MaxValue / (long double)m_frequency.QuadPart * MICRO), (*p)->m_call_count);
		}

		_ftprintf_s(output, _TEXT("  -----------------------------------------------------------------------------------------------------------------------\n"));
		fclose(output);

		m_file[0] = '\0';
	}
}

void PROFILE::Delete()
{
	if (GetAsyncKeyState('L') & 0x0001)
	{
		if (m_is_lock == true)
			_tprintf(_TEXT("-----U N L O C K-----\n"));
		else
			_tprintf(_TEXT("-----L O C K-----\n"));

		m_is_lock = !m_is_lock;
	}

	if (m_is_lock == false && (GetAsyncKeyState(VK_END) & 0x0001))
		node_list.clear();
}
