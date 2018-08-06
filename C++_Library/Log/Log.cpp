#include "Precompile.h"
#include "Log.h"

#include <time.h>
#include <stdio.h>
#include <direct.h>	
#include <strsafe.h>

BYTE C_Log::m_log_level = LOG_LEVEL_SYSTEM;
TCHAR C_Log::m_dir_path[50];
SRWLOCK C_Log::m_log_srwLock; SRWLOCK C_Log::m_binary_log_srwLock;

volatile LONGLONG C_Log::m_count = 0;

C_Log::C_Log()
{
	InitializeSRWLock(&m_log_srwLock);
	InitializeSRWLock(&m_binary_log_srwLock);	
}

DWORD C_Log::S_MakeDirectory(const char* pa_path)
{
	MultiByteToWideChar(CP_UTF8, 0, pa_path, strlen(pa_path), m_dir_path, _countof(m_dir_path));

	int result = _mkdir(pa_path);
	if (result == -1)
	{
		int error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
			return error;
	}

	return 0;
}

void C_Log::S_PrintLog(int pa_line, BYTE pa_log_level, TCHAR* pa_action, TCHAR* pa_server, initializer_list<string> pa_error_str)
{
	AcquireSRWLockExclusive(&m_log_srwLock);

	FILE *input;
	TCHAR output_buffer[100], log_buffer[30], file_name[100];
	
	time_t cur_time;
	struct tm time_info;
	
	ZeroMemory(&time_info, sizeof(time_info));
	cur_time = time(NULL);
	localtime_s(&time_info, &cur_time);

	StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_Log.txt"), m_dir_path, pa_action, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
	_tfopen_s(&input, file_name, _TEXT("a"));

	switch (pa_log_level)
	{
		case LOG_LEVEL_SYSTEM:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----SYSTEM"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;

		case LOG_LEVEL_ERROR:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----ERROR"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;
	
		case LOG_LEVEL_WARNING:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----WARNING"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;
	
		case LOG_LEVEL_DEBUG:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----DEBUG"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;
	}

	StringCchPrintf(output_buffer, 100, _TEXT("-----[%s]   [Line:%d] %04d/%02d/%02d_%02d:%02d:%02d   [%I64u%]-----"), pa_server, pa_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec, InterlockedIncrement64(&m_count));
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	for (auto iter : pa_error_str)
		fprintf(input, "%s\n", iter.c_str());

	fclose(input);
	ReleaseSRWLockExclusive(&m_log_srwLock);
}

void C_Log::S_BinaryLog(int pa_line, TCHAR* pa_action, TCHAR* pa_server, char* pa_str, int pa_size)
{
	AcquireSRWLockExclusive(&m_binary_log_srwLock);
	
	FILE *input;
	TCHAR output_buffer[100], file_name[50];

	time_t cur_time;
	struct tm time_info;

	ZeroMemory(&time_info, sizeof(time_info));
	cur_time = time(NULL);
	localtime_s(&time_info, &cur_time);

	StringCchPrintf(file_name, 50, _TEXT("%s/%s_%04d%02d%02d_BinaryLog.txt"), m_dir_path, pa_action, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
	_tfopen_s(&input, file_name, _TEXT("a"));

	StringCchPrintf(output_buffer, 100, _TEXT("-----[%s]   [Line:%d] %04d/%02d/%02d_%02d:%02d:%02d   [%I64u%]-----"), pa_server, pa_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec, InterlockedIncrement64(&m_count));
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	fprintf(input, "Binary : [");
	for (int i = 0; i < pa_size; i++)
		fprintf(input, "%#x|", pa_str[i]);
	fprintf(input, "]\n");

	fclose(input);
	ReleaseSRWLockExclusive(&m_binary_log_srwLock);
}