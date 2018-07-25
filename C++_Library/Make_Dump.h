#ifndef Dump_Info
#define Dump_Info

#include <Windows.h>

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