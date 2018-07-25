#ifndef Log_Info
#define Log_Info

#define LOG_LEVEL_SYSTEM  0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_DEBUG   3

#include <Windows.h>

/*
 * 1. 실행에서 메인이될 cpp 파일의 전역으로 Log 객체 생성 ( SRWLock 초기화 목적 )
 * 2. 각 실행파일에서 남겨질 로그가 저장될 폴더를 _MAKEDIR 매크로를 이용하여 생성
 * 3. 로그남길 경우 _LOG 매크로를 호출 [ Action: 매크로를 호출하는 cpp 파일명, Server: 매크로를 호출하는 서버 종류 ] 
 * 4. 바이너리 로그를 남길 경우 _BinaryLOG 매크로를 호출 [ Action: 매크로를 호출하는 cpp 파일명, Server: 매크로를 호출하는 서버 종류, Size: 바이너리 크기 ]
*/

class LOG
{
private:
	static SRWLOCK m_log_srwLock;
	static volatile LONGLONG m_count;

public:
	static BYTE m_log_level;
	
	LOG();
	static void MakeDirectory(const char* p_path);
	static void PrintLog(int p_line, BYTE p_log_level, TCHAR* p_action, TCHAR* p_server, initializer_list<string> p_str);
	static void BinaryLog(int p_line, TCHAR* p_action, TCHAR* p_server, char* p_str, int p_size);
};

#define _MAKEDIR(p_path)	LOG::MakeDirectory(p_path);

#define _LOG(Line, LogLevel, Action, Server, Str)						\
	do																	\
	{																	\
			if(LogLevel <= LOG::m_log_level)							\
				LOG::PrintLog(Line, LogLevel, Action, Server, Str);		\
	}while (0)															\

#define _BinaryLOG(Line, Action, Server, Str, Size)						\
	do																	\
	{																	\
		LOG::BinaryLog(Line, Action, Server, Str, Size);				\
	}while (0)															\

#endif