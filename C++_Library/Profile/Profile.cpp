#include "Precompile.h"
#include "Profile.h"

#include "Log/Log.h"

#include <time.h>
#include <stdlib.h>
#include <strsafe.h>

using namespace Olbbemi;

LONG C_Profile::s_index = 0;
DWORD C_Profile_Chunk::s_tls_index = 0;
C_Profile_Chunk** C_Profile::s_chunk = nullptr;

C_Profile::C_Profile(int pa_thread_count) : m_is_lock_on(true), m_file(_TEXT("Profile_"))
{
	s_index = -1;	m_thread_count = pa_thread_count;
	QueryPerformanceFrequency(&m_frequency);

	s_chunk = new C_Profile_Chunk*[pa_thread_count];

	for (int i = 0; i < pa_thread_count; i++)
		s_chunk[i] = new C_Profile_Chunk;
}

C_Profile::~C_Profile()
{
	for (int i = 0; i < m_thread_count; i++)
		delete s_chunk[i];

	delete s_chunk;
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

void C_Profile::M_Save()
{
	FILE *lo_output = nullptr;

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
	else if (m_is_lock_on == false && (GetAsyncKeyState(VK_HOME) & 0x0001))
	{
		int lo_thread_count = m_thread_count;
		M_GetTime();

		_tfopen_s(&lo_output, m_file, _TEXT("w, ccs=UTF-16LE"));

		for (int i = 0; i < lo_thread_count; i++)
		{
			_ftprintf_s(lo_output, _TEXT("  -----------------------------------------------------------------------------------------------------------------------\n"));
			_ftprintf_s(lo_output, _TEXT("#     ThreadID    #            Name            #       Average       #       Min_Value       #       Max_Value       #       Count       #\n"));

			for (auto p = s_chunk[i]->m_node_list.begin(); p != s_chunk[i]->m_node_list.end(); p++)
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

				_ftprintf_s(lo_output, _TEXT("#%d#%30s#%22.4Lf#%24.4Lf#%24.4Lf#%18lld#\n"), s_chunk[i]->m_thread_id, (*p)->m_function_name, ((long double)(*p)->m_total_time / (long double)(*p)->m_call_count / (long double)m_frequency.QuadPart * MICRO), ((long double)MinValue / (long double)m_frequency.QuadPart * MICRO), ((long double)MaxValue / (long double)m_frequency.QuadPart * MICRO), (*p)->m_call_count);
			}

			_ftprintf_s(lo_output, _TEXT("  -----------------------------------------------------------------------------------------------------------------------\n"));
		}

		fclose(lo_output);
		m_file[0] = '\0';
	}
}

C_Profile_Chunk* C_Profile::S_Alloc()
{
	LONG lo_index = InterlockedIncrement(&s_index);
	return s_chunk[lo_index];
}

C_Profile_Chunk::C_Node::C_Node(const PTCHAR pa_str, __int64 pa_time)
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

C_Profile_Chunk::C_Profile_Chunk()
{
	s_tls_index = TlsAlloc();
	if (s_tls_index == TLS_OUT_OF_INDEXES)
	{
		TCHAR lo_action[] = _TEXT("Profile"), lo_server[] = _TEXT("NONE");
		ST_Log lo_log({ "TlsAlloc Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}
}

C_Profile_Chunk::~C_Profile_Chunk()
{
	int size = m_node_list.size();
	for (int i = 0; i < size; i++)
		delete m_node_list[i];

	m_node_list.clear();
}

void Olbbemi::Profile_Begin(const PTCHAR pa_str, int pa_line)
{
	void* lo_tls_ptr = TlsGetValue(C_Profile_Chunk::s_tls_index);
	if (lo_tls_ptr == nullptr)
	{
		lo_tls_ptr = (C_Profile_Chunk*)C_Profile::S_Alloc();
		BOOL lo_check = TlsSetValue(C_Profile_Chunk::s_tls_index, lo_tls_ptr);
		if (lo_check == 0)
		{
			TCHAR lo_action[] = _TEXT("Profile"), lo_server[] = _TEXT("NONE");
			ST_Log lo_log({ "TlsSetValue Error Code: " + to_string(GetLastError()) });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
		}

		((C_Profile_Chunk*)lo_tls_ptr)->m_thread_id = GetCurrentThreadId();
	}

	C_Profile_Chunk::E_Status lo_flag = C_Profile_Chunk::E_Status::normal;
	for (auto p = ((C_Profile_Chunk*)lo_tls_ptr)->m_node_list.begin(); p != ((C_Profile_Chunk*)lo_tls_ptr)->m_node_list.end(); p++)
	{
		if (!_tcscmp((*p)->m_function_name, pa_str))
		{
			if ((*p)->m_start_time != 0)
				lo_flag = C_Profile_Chunk::E_Status::fail;
			else
			{
				lo_flag = C_Profile_Chunk::E_Status::success;
				QueryPerformanceCounter(&((C_Profile_Chunk*)lo_tls_ptr)->m_begin_count);
				(*p)->m_start_time = ((C_Profile_Chunk*)lo_tls_ptr)->m_begin_count.QuadPart;
			}

			break;
		}
	}

	if (lo_flag == C_Profile_Chunk::E_Status::fail)
	{
		_tprintf(_TEXT("%d : Need to End Function"), pa_line);
		throw;
	}
	else if (lo_flag == C_Profile_Chunk::E_Status::normal)
	{
		QueryPerformanceCounter(&((C_Profile_Chunk*)lo_tls_ptr)->m_begin_count);
		C_Profile_Chunk::C_Node *lo_new_node = new C_Profile_Chunk::C_Node(pa_str, ((C_Profile_Chunk*)lo_tls_ptr)->m_begin_count.QuadPart);
		((C_Profile_Chunk*)lo_tls_ptr)->m_node_list.push_back(lo_new_node);
	}
}

void Olbbemi::Profile_End(const PTCHAR pa_str)
{
	void* lo_tls_ptr = TlsGetValue(C_Profile_Chunk::s_tls_index);

	QueryPerformanceCounter(&((C_Profile_Chunk*)lo_tls_ptr)->m_end_count);
	for (auto p = ((C_Profile_Chunk*)lo_tls_ptr)->m_node_list.begin(); p != ((C_Profile_Chunk*)lo_tls_ptr)->m_node_list.end(); p++)
	{
		if (!_tcscmp((*p)->m_function_name, pa_str))
		{
			(*p)->m_call_count++;
			(*p)->m_total_time += ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time;

			if ((*p)->m_min_count != 3 && (*p)->m_min_value[(*p)->m_min_count] > ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time)
				(*p)->m_min_value[(*p)->m_min_count++] = ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time;
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

				if (((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time < comp_value)
					(*p)->m_min_value[lo_index] = ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time;
			}

			if ((*p)->m_max_count != 3 && (*p)->m_max_value[(*p)->m_max_count] < ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time)
				(*p)->m_max_value[(*p)->m_max_count++] = ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time;
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

				if (((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time > comp_value)
					(*p)->m_max_value[lo_index] = ((C_Profile_Chunk*)lo_tls_ptr)->m_end_count.QuadPart - (*p)->m_start_time;
			}

			(*p)->m_start_time = 0;
			break;
		}
	}
}