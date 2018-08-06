#ifndef Log_Info
#define Log_Info

#define LOG_LEVEL_SYSTEM  0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_DEBUG   3

#include <Windows.h>

#include <string>
#include <initializer_list>

using namespace std;

/*
 * 1. 실행에서 메인이될 cpp 파일의 전역으로 Log 객체 생성 ( SRWLock 초기화 목적 )
 * 2. 각 실행파일이 실행될 때 남겨질 로그가 저장될 폴더를 _MAKEDIR 매크로를 이용하여 생성
 * 3. 로그남길 경우 _LOG 매크로를 호출 [ Action: 매크로를 호출하는 cpp 파일명, Server: 매크로를 호출하는 서버 종류 ] 
 * 4. 바이너리 로그를 남길 경우 _BinaryLOG 매크로를 호출 [ Action: 매크로를 호출하는 cpp 파일명, Server: 매크로를 호출하는 서버 종류, Size: 바이너리 크기 ]
*/

class C_Log
{
private:
	static TCHAR m_dir_path[50];
	static SRWLOCK m_log_srwLock, m_binary_log_srwLock;
	static volatile LONGLONG m_count;

public:
	static BYTE m_log_level;
	
	C_Log();
	static DWORD S_MakeDirectory(const char* pa_path);
	static void S_PrintLog(int pa_line, BYTE pa_log_level, TCHAR* pa_action, TCHAR* pa_server, initializer_list<string> pa_error_str);
	static void S_BinaryLog(int pa_line, TCHAR* pa_action, TCHAR* pa_server, char* pa_str, int pa_size);
};

#define _MAKEDIR(pa_path)	C_Log::MakeDirectory(pa_path);

#define _LOG(pa_line, pa_log_level, pa_action, pa_server, pa_error_str)						\
	do																						\
	{																						\
			if(pa_log_level <= C_Log::m_log_level)											\
				C_Log::S_PrintLog(pa_line, pa_log_level, pa_action, pa_server, pa_error_str);	\
	}while (0)																				\

#define _BinaryLOG(pa_line, pa_action, pa_server, pa_error_str, pa_size)		\
	do																			\
	{																			\
		C_Log::S_BinaryLog(pa_line, pa_action, pa_server, pa_error_str, pa_size);	\
	}while (0)																	\

#endif