#include "Precompile.h"
#include "Log.h"

#include <time.h>
#include <stdio.h>
#include <direct.h>	
#include <strsafe.h>

BYTE C_Log::m_log_level = LOG_LEVEL_DEBUG;
TCHAR C_Log::m_dir_path[50];
SRWLOCK C_Log::m_log_srwLock; SRWLOCK C_Log::m_binary_log_srwLock;

C_Dump g_dump;
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

void C_Log::S_PrintLog(int pa_line, BYTE pa_log_level, TCHAR* pa_action, TCHAR* pa_server, WORD pa_str_count, string pa_error_str[])
{
	AcquireSRWLockExclusive(&m_log_srwLock);

	FILE *input;
	TCHAR output_buffer[100], log_buffer[30], file_name[100];
	
	time_t cur_time;
	struct tm time_info;
	
	ZeroMemory(&time_info, sizeof(time_info));
	cur_time = time(NULL);
	localtime_s(&time_info, &cur_time);

	switch (pa_log_level)
	{
		case LOG_LEVEL_POWER:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----POWER"));
			StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_PowerLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
			break;

		case LOG_LEVEL_SYSTEM:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----SYSTEM"));
			StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_SystemLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
			break;

		case LOG_LEVEL_QUERY:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----Query"));
			StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_QueryLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
			break;

		case LOG_LEVEL_ERROR:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----ERROR"));
			StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_ErrorLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
			break;
	
		case LOG_LEVEL_WARNING:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----WARNING"));
			StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_WarningLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
			break;
	
		case LOG_LEVEL_DEBUG:
			StringCchPrintf(log_buffer, 30, _TEXT("Status: -----DEBUG"));
			StringCchPrintf(file_name, 100, _TEXT("%s/%s_%04d%02d%02d_DebugLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
			break;
	}

	StringCchPrintf(output_buffer, 100, _TEXT("-----[%s]   [Line:%d] %04d/%02d/%02d_%02d:%02d:%02d   [%I64u%]-----"), pa_action, pa_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec, InterlockedIncrement64(&m_count));

	_tfopen_s(&input, file_name, _TEXT("a"));
	_ftprintf(input, _TEXT("%s\n"), log_buffer);
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	for (int i = 0; i < pa_str_count; i++)
		fprintf(input, "%s\n", pa_error_str[i].c_str());
	fprintf(input, "\n");

	fclose(input);
	ReleaseSRWLockExclusive(&m_log_srwLock);

	if (pa_log_level == LOG_LEVEL_SYSTEM || pa_log_level == LOG_LEVEL_QUERY)
		g_dump.S_MakeCrash();
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

	StringCchPrintf(file_name, 50, _TEXT("%s/%s_%04d%02d%02d_BinaryLog.txt"), m_dir_path, pa_server, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
	_tfopen_s(&input, file_name, _TEXT("a"));

	StringCchPrintf(output_buffer, 100, _TEXT("-----[%s]   [Line:%d] %04d/%02d/%02d_%02d:%02d:%02d   [%I64u%]-----"), pa_action, pa_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec, InterlockedIncrement64(&m_count));
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	fprintf(input, "Binary : [");
	for (int i = 0; i < pa_size; i++)
		fprintf(input, "%#x|", pa_str[i]);
	fprintf(input, "]\n");

	fclose(input);
	ReleaseSRWLockExclusive(&m_binary_log_srwLock);
}