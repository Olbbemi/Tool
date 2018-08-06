#include "Precompile.h"
#include "Profile.h"

#include <time.h>
#include <stdlib.h>
#include <strsafe.h>

using namespace Olbbemi;

C_Profile::C_Node::C_Node(const PTCHAR pa_str, __int64 pa_time)
{
	m_min_count = m_max_count = 0;
	m_call_count = m_total_time = 0;
	m_start_time = pa_time;
	StringCchCopy(m_function_name, _countof(m_function_name), pa_str);

	for (int i = 0; i < 3; i++)
	{
		m_max_value[i] = 0;
		m_min_value[i] = 0x7FFFFFFFFFFFFFFF;
	}
}

C_Profile::C_Profile() : m_is_lock_on(true), m_file(_TEXT("Profile_"))
{
	QueryPerformanceFrequency(&m_frequency);
}

void C_Profile::M_GetTime()
{
	TCHAR lo_date[20], lo_tail[5] = _TEXT(".txt");
	time_t lo_timer = time(NULL);

	struct tm lo_time;
	localtime_s(&lo_time, &lo_timer);

	StringCchPrintf(lo_date, 20, _TEXT("%4d_%02d%02d_%02d%02d%02d"), lo_time.tm_year + 1900, lo_time.tm_mon + 1, lo_time.tm_mday, lo_time.tm_hour, lo_time.tm_min, lo_time.tm_sec);
	StringCchCat(m_file, _countof(m_file), lo_date);
	StringCchCat(m_file, _countof(m_file), lo_tail);
}

void Olbbemi::Profile_Begin(C_Profile *pa_obj, const PTCHAR pa_str, int pa_line)
{
	C_Profile::E_Status lo_flag = C_Profile::E_Status::normal;
	for (auto p = pa_obj->m_node_list.begin(); p != pa_obj->m_node_list.end(); p++)
	{
		if (!_tcscmp((*p)->m_function_name, pa_str))
		{
			if ((*p)->m_start_time != 0)
				lo_flag = C_Profile::E_Status::fail;
			else
			{
				lo_flag = C_Profile::E_Status::success;
				QueryPerformanceCounter(&pa_obj->m_begin_count);
				(*p)->m_start_time = pa_obj->m_begin_count.QuadPart;
			}

			break;
		}
	}

	if (lo_flag == C_Profile::E_Status::fail)
	{
		_tprintf(_TEXT("%d : Need to End Function"), pa_line);
		throw;
	}
	else if (lo_flag == C_Profile::E_Status::normal)
	{
		QueryPerformanceCounter(&pa_obj->m_begin_count);
		C_Profile::C_Node *lo_new_node = new C_Profile::C_Node(pa_str, pa_obj->m_begin_count.QuadPart);
		pa_obj->m_node_list.push_back(lo_new_node);
	}
}

void Olbbemi::Profile_End(C_Profile *pa_obj, const PTCHAR pa_str)
{
	QueryPerformanceCounter(&pa_obj->m_end_count);
	for (auto p = pa_obj->m_node_list.begin(); p != pa_obj->m_node_list.end(); p++)
	{
		if (!_tcscmp((*p)->m_function_name, pa_str))
		{
			(*p)->m_call_count++;
			(*p)->m_total_time += pa_obj->m_end_count.QuadPart - (*p)->m_start_time;

			if ((*p)->m_min_count != 3 && (*p)->m_min_value[(*p)->m_min_count] > pa_obj->m_end_count.QuadPart - (*p)->m_start_time)
				(*p)->m_min_value[(*p)->m_min_count++] = pa_obj->m_end_count.QuadPart - (*p)->m_start_time;
			else if ((*p)->m_min_count == 3)
			{
				int lo_index = 0;
				__int64 comp_value = (*p)->m_min_value[0];
				for (int i = 1; i < 3; i++)
				{
					if (comp_value < (*p)->m_min_value[i])
					{
						comp_value = (*p)->m_min_value[i];
						lo_index = i;
					}
				}

				if (pa_obj->m_end_count.QuadPart - (*p)->m_start_time < comp_value)
					(*p)->m_min_value[lo_index] = pa_obj->m_end_count.QuadPart - (*p)->m_start_time;
			}

			if ((*p)->m_max_count != 3 && (*p)->m_max_value[(*p)->m_max_count] < pa_obj->m_end_count.QuadPart - (*p)->m_start_time)
				(*p)->m_max_value[(*p)->m_max_count++] = pa_obj->m_end_count.QuadPart - (*p)->m_start_time;
			else if ((*p)->m_max_count == 3)
			{
				int lo_index = 0;
				__int64 comp_value = (*p)->m_max_value[0];
				for (int i = 1; i < 3; i++)
				{
					if (comp_value >(*p)->m_max_value[i])
					{
						comp_value = (*p)->m_max_value[i];
						lo_index = i;
					}
				}

				if (pa_obj->m_end_count.QuadPart - (*p)->m_start_time > comp_value)
					(*p)->m_max_value[lo_index] = pa_obj->m_end_count.QuadPart - (*p)->m_start_time;
			}

			(*p)->m_start_time = 0;
			break;
		}
	}
}

void C_Profile::M_Save()
{
	FILE *lo_output;

	if (m_is_lock_on == false && (GetAsyncKeyState(VK_HOME) & 0x0001))
	{
		C_Profile::M_GetTime();

		_tfopen_s(&lo_output, m_file, _TEXT("w, ccs=UTF-16LE"));
		_ftprintf_s(lo_output, _TEXT("  -----------------------------------------------------------------------------------------------------------------------\n"));
		_ftprintf_s(lo_output, _TEXT("#            Name            #       Average       #       Min_Value       #       Max_Value       #       Count       #\n"));

		for (auto p = m_node_list.begin(); p != m_node_list.end(); p++)
		{
			__int64 MaxValue = (*p)->m_max_value[0], MinValue = (*p)->m_min_value[0];

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

			_ftprintf_s(lo_output, _TEXT("#%30s#%22.4Lf#%24.4Lf#%24.4Lf#%18lld#\n"), (*p)->m_function_name, ((long double)(*p)->m_total_time / (long double)(*p)->m_call_count / (long double)m_frequency.QuadPart * MICRO), ((long double)MinValue / (long double)m_frequency.QuadPart * MICRO), ((long double)MaxValue / (long double)m_frequency.QuadPart * MICRO), (*p)->m_call_count);
		}

		_ftprintf_s(lo_output, _TEXT("  -----------------------------------------------------------------------------------------------------------------------\n"));
		fclose(lo_output);

		m_file[0] = '\0';
	}
}

void C_Profile::M_Delete()
{
	if (GetAsyncKeyState('L') & 0x0001)
	{
		m_is_lock_on = true;
		_tprintf(_TEXT("-----L O C K-----\n"));
	}
	else if (GetAsyncKeyState('U') & 0x0001)
	{
		m_is_lock_on = false;
		_tprintf(_TEXT("-----U N L O C K-----\n"));
	}

	if (m_is_lock_on == false && (GetAsyncKeyState(VK_END) & 0x0001))
		m_node_list.clear();
}
