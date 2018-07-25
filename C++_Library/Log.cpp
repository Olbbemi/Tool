#include "Precompile.h"
#include "Log.h"

#include <time.h>
#include <stdio.h>
#include <direct.h>	
#include <stdarg.h>
#include <strsafe.h>

SRWLOCK LOG::m_log_srwLock;
BYTE LOG::m_log_level = LOG_LEVEL_ERROR;
volatile LONGLONG LOG::m_count = 0;

LOG::LOG()
{
	InitializeSRWLock(&m_log_srwLock);
}

void LOG::MakeDirectory(const char* p_path)
{
	int result = _mkdir(p_path);
	if (result == -1)
	{
		int error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
		{
			TCHAR type[] = _TEXT("LOG"), server[] = _TEXT("NONE");
			initializer_list<string> str = { "CreateDirectory Error Code: " + to_string(error) };
			PrintLog(__LINE__, LOG_LEVEL_ERROR, type, server, str);
		}
	}
}

void LOG::PrintLog(int p_line, BYTE p_log_level, TCHAR* p_action, TCHAR* p_server, initializer_list<string> p_str)
{
	AcquireSRWLockExclusive(&m_log_srwLock);

	FILE *input;
	TCHAR output_buffer[100], log_buffer[30], file_name[100];
	
	time_t cur_time;
	struct tm time_info;
	
	ZeroMemory(&time_info, sizeof(time_info));
	cur_time = time(NULL);
	localtime_s(&time_info, &cur_time);

	StringCchPrintf(file_name, 100, _TEXT("%s_%04d%02d%02d_Log.txt"), p_action, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
	_tfopen_s(&input, file_name, _TEXT("a"));

	switch (p_log_level)
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

	StringCchPrintf(output_buffer, 100, _TEXT("-----[%s]   [Line:%d] %04d/%02d/%02d_%02d:%02d:%02d   [%I64u%]-----"), p_server, p_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec, InterlockedIncrement64(&m_count));
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	for (auto iter : p_str)
		fprintf(input, "%s\n", iter.c_str());

	fclose(input);
	ReleaseSRWLockExclusive(&m_log_srwLock);
}

void LOG::BinaryLog(int p_line, TCHAR* p_action, TCHAR* p_server, char* p_str, int p_size)
{
	AcquireSRWLockExclusive(&m_log_srwLock);
	
	FILE *input;
	TCHAR output_buffer[100], file_name[50];

	time_t cur_time;
	struct tm time_info;

	ZeroMemory(&time_info, sizeof(time_info));
	cur_time = time(NULL);
	localtime_s(&time_info, &cur_time);

	StringCchPrintf(file_name, 50, _TEXT("%s_%04d%02d%02d_BinaryLog.txt"), p_action, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
	_tfopen_s(&input, file_name, _TEXT("a"));

	StringCchPrintf(output_buffer, 100, _TEXT("-----[%s]   [Line:%d] %04d/%02d/%02d_%02d:%02d:%02d   [%I64u%]-----"), p_server, p_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec, InterlockedIncrement64(&m_count));
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	fprintf(input, "Binary : [");
	for (int i = 0; i < p_size; i++)
		fprintf(input, "%#x ", p_str[i]);
	fprintf(input, "]\n");

	fclose(input);
	ReleaseSRWLockExclusive(&m_log_srwLock);
}