#ifndef Log_Info
#define Log_Info

#define LOG_LEVEL_SYSTEM  0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_DEBUG   3

#include <Windows.h>

/*
 * 1. ���࿡�� �����̵� cpp ������ �������� Log ��ü ���� ( SRWLock �ʱ�ȭ ���� )
 * 2. �� �������Ͽ��� ������ �αװ� ����� ������ _MAKEDIR ��ũ�θ� �̿��Ͽ� ����
 * 3. �α׳��� ��� _LOG ��ũ�θ� ȣ�� [ Action: ��ũ�θ� ȣ���ϴ� cpp ���ϸ�, Server: ��ũ�θ� ȣ���ϴ� ���� ���� ] 
 * 4. ���̳ʸ� �α׸� ���� ��� _BinaryLOG ��ũ�θ� ȣ�� [ Action: ��ũ�θ� ȣ���ϴ� cpp ���ϸ�, Server: ��ũ�θ� ȣ���ϴ� ���� ����, Size: ���̳ʸ� ũ�� ]
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