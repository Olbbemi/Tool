#include "Precompile.h"
#include "Log.h"

#include <stdarg.h>
#include <time.h>

BYTE LOG::m_log_level = LOG_LEVEL_ERROR;

void LOG::PrintLog(int p_line, BYTE p_log_level, int p_fix_data, ...)
{
	TCHAR output_buffer[100], log_buffer[30], file_name[50];
	FILE *input;
	
	va_list args;
	time_t cur_time;
	struct tm time_info;
	
	ZeroMemory(&time_info, sizeof(time_info));
	cur_time = time(NULL);
	localtime_s(&time_info, &cur_time);

	_stprintf_s(file_name, 50, _TEXT("%04d%02d%02d_Log.txt"), time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);
	_tfopen_s(&input, file_name, _TEXT("a"));

	switch (p_log_level)
	{
		case LOG_LEVEL_ERROR:
			_stprintf_s(log_buffer, 30, _TEXT("Status: -----ERROR"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;
	
		case LOG_LEVEL_WARNING:
			_stprintf_s(log_buffer, 30, _TEXT("Status: -----WARNING"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;
	
		case LOG_LEVEL_DEBUG:
			_stprintf_s(log_buffer, 30, _TEXT("Status: -----DEBUG"));
			_ftprintf(input, _TEXT("%s\n"), log_buffer);
			break;
	}

	_stprintf_s(output_buffer, 100, _TEXT("-----[ Line:%d ]%04d/%02d/%02d_%02d:%02d:%02d-----"), p_line, time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday, time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
	_ftprintf(input, _TEXT("%s\n"), output_buffer);

	va_start(args, p_fix_data);
	for (int i = 0; i < p_fix_data; i++)
		_ftprintf(input, _TEXT("%s\n"), va_arg(args, char*));
	va_end(args);

	fclose(input);
}