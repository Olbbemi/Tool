#ifndef Log_Info
#define Log_Info

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_DEBUG 2

#include <Windows.h>

class LOG
{
public:
	static BYTE m_log_level;
	static void PrintLog(int p_line, BYTE p_log_level, int p_fix_data, ...);
};

#define _LOG(Line, LogLevel, fix, ...)									\
	do																	\
	{																	\
			if(LogLevel <= LOG::m_log_level)							\
				LOG::PrintLog(Line, LogLevel, fix, ##__VA_ARGS__);		\
	}while (0)															\

#endif