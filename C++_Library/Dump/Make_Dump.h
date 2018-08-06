#ifndef Dump_Info
#define Dump_Info

#include <Windows.h>

/*
 * 1. 프로젝트마다 메인.cpp 파일 전역변수로 객체 생성
 * 2. STL 함수는 덤프로 남겨지나 확인하기 어려움
 * 3. 해당 파일로 탐지 못하는 에러들이 있으므로 항상 외부덤프도 같이 사용
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