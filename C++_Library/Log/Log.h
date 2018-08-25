#ifndef Log_Info
#define Log_Info

#define LOG_LEVEL_SYSTEM  0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_DEBUG   3

#include "Dump/Make_Dump.h"

#include <Windows.h>

#include <string>
#include <initializer_list>

using namespace std;
using namespace Olbbemi;

/**----------------------------------------------------------------------------------------------------------------------------------------------------
  *  1. ���࿡�� �����̵� cpp ������ �������� Log ��ü ���� ( SRWLock �ʱ�ȭ ���� )
  *  2. �� ���������� ����� �� ������ �αװ� ����� ������ _MAKEDIR ��ũ�θ� �̿��Ͽ� ����
  *  3. �α׳��� ��� _LOG ��ũ�θ� ȣ�� [ Action: ��ũ�θ� ȣ���ϴ� cpp ���ϸ�, Server: ��ũ�θ� ȣ���ϴ� ���� ���� ] 
  *  4. �α׸� ���ܾ��ϴ� ���Ͽ����� ST_Log ��ü�� �����ϰ� �α� �Լ� ���ڷ� ����� �� �α� �Լ� ȣ��
  *  5. ���̳ʸ� �α׸� ���� ��� _BinaryLOG ��ũ�θ� ȣ�� [ Action: ��ũ�θ� ȣ���ϴ� cpp ���ϸ�, Server: ��ũ�θ� ȣ���ϴ� ���� ����, Size: ���̳ʸ� ũ�� ]
  *  6. System_Level �α��� ��� Dump Ŭ������ �̿��Ͽ� Crash ���� 
  *----------------------------------------------------------------------------------------------------------------------------------------------------*/

 /**------------------------------------------------
   * ���Ͽ� ��µ� �α� ������ ������ ����ü
   * initializer_list ��ü�� �̿��Ͽ� �������ڸ� ����
   * count ������ �������� ������ �ǹ�
   *------------------------------------------------*/
struct ST_Log
{
	int count;
	string log_str[20];

	ST_Log(initializer_list<string> pa_log) : count(0)
	{
		for (auto p : pa_log)
			log_str[count++] = p;
	}
};

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
	static void S_PrintLog(int pa_line, BYTE pa_log_level, TCHAR* pa_action, TCHAR* pa_server, WORD pa_str_count, string pa_error_str[]);
	static void S_BinaryLog(int pa_line, TCHAR* pa_action, TCHAR* pa_server, char* pa_str, int pa_size);
};

#define _MAKEDIR(pa_path)	C_Log::S_MakeDirectory(pa_path);

#define _LOG(pa_line, pa_log_level, pa_action, pa_server, pa_str_count, pa_error_str)						\
	do																										\
	{																										\
			if(pa_log_level <= C_Log::m_log_level)															\
				C_Log::S_PrintLog(pa_line, pa_log_level, pa_action, pa_server, pa_str_count, pa_error_str);	\
	}while (0)																								\

#define _BinaryLOG(pa_line, pa_action, pa_server, pa_error_str, pa_size)			\
	do																				\
	{																				\
		C_Log::S_BinaryLog(pa_line, pa_action, pa_server, pa_error_str, pa_size);	\
	}while (0)																		\

extern C_Dump g_dump;

#endif