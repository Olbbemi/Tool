#include "Precompile.h"
#include "Make_Dump.h"
#include "Log.h"

#include <DbgHelp.h>
#include <signal.h>
#include <Psapi.h>
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>

#pragma comment(lib,"dbgHelp.Lib")

using namespace Olbbemi;

volatile long CrashDump::_DumpCount = 0;

CrashDump::CrashDump()
{
	_invalid_parameter_handler oldHandler, newHandler;

	newHandler = UnhandledHandler;
	oldHandler = _set_invalid_parameter_handler(newHandler);
	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);

	_set_purecall_handler(PureCallHandler);

	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	signal(SIGABRT, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGILL, SignalHandler);
	signal(SIGFPE, SignalHandler);
	signal(SIGSEGV, SignalHandler);
	signal(SIGTERM, SignalHandler);

	SetHandlerDump();
}

void CrashDump::SetHandlerDump()
{
	SetUnhandledExceptionFilter(PrintException);
}

void CrashDump::UnhandledHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* filfile, unsigned int line, uintptr_t pReserved)
{
	int *crash_point = nullptr;
	*crash_point = -1;
}

void CrashDump::MakeCrash()
{
	int *crash_point = nullptr;
	*crash_point = -1;
}

void CrashDump::SignalHandler(int p_error)
{
	int *crash_point = nullptr;
	*crash_point = -1;
}

void CrashDump::PureCallHandler()
{
	int *crash_point = nullptr;
	*crash_point = -1;
}

LONG WINAPI CrashDump::PrintException(__in PEXCEPTION_POINTERS pExceptionPointer)
{
	TCHAR filename[255];
	SYSTEMTIME system_curtime;

	GetLocalTime(&system_curtime);
	long DumpCount = InterlockedIncrement(&_DumpCount);
	
	StringCchPrintf(filename, 255, _TEXT("Dump_%d%02d%02d_%02d%02d%02d_%d.dmp"), system_curtime.wYear, system_curtime.wMonth, system_curtime.wDay, system_curtime.wHour, system_curtime.wMinute, system_curtime.wSecond, GetCurrentThreadId());

	HANDLE dumpfile = ::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dumpfile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInformation;

		minidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
		minidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		minidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpfile, MiniDumpWithFullMemory, &minidumpExceptionInformation, NULL, NULL);
		CloseHandle(dumpfile);
	}
	else
	{
		TCHAR action[] = _TEXT("DUMP"), server[] = _TEXT("NONE");
		initializer_list<string> str = { "Createfile Error Code:" + GetLastError() };
		_LOG(__LINE__, LOG_LEVEL_ERROR, action, server, str);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}