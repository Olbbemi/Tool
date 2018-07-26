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
	class CrashDump
	{
	private:
		static volatile long _DumpCount;
		
	public:
		CrashDump();

		static void MakeCrash();
		static void SignalHandler(int p_error);
		static void PureCallHandler();
		static LONG WINAPI PrintException(__in PEXCEPTION_POINTERS pExceptionPointer);

		static void UnhandledHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* filfile, unsigned int line, uintptr_t pReserved);

		static void SetHandlerDump();
	};
}

#endif