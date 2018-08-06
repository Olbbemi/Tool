#ifndef Dump_Info
#define Dump_Info

#include <Windows.h>

/*
 * 1. ������Ʈ���� ����.cpp ���� ���������� ��ü ����
 * 2. STL �Լ��� ������ �������� Ȯ���ϱ� �����
 * 3. �ش� ���Ϸ� Ž�� ���ϴ� �������� �����Ƿ� �׻� �ܺδ����� ���� ���
 */

namespace Olbbemi
{
	class C_Dump
	{
	private:
		static volatile long s_DumpCount;
		
	public:
		C_Dump();

		static void S_MakeCrash();
		static void S_SignalHandler(int pa_error);
		static void S_PureCallHandler();
		static LONG WINAPI S_PrintException(__in PEXCEPTION_POINTERS pa_ExceptionPointer);

		static void S_UnhandledHandler(const wchar_t* pa_expression, const wchar_t* pa_function, const wchar_t* pa_filfile, unsigned int pa_line, uintptr_t pa_Reserved);

		static void S_SetHandlerDump();
	};
}

#endif