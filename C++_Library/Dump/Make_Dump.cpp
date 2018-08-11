#include "Precompile.h"
#include "Log/Log.h"
#include "Make_Dump.h"

#include <DbgHelp.h>
#include <signal.h>
#include <Psapi.h>
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>

#pragma comment(lib,"dbgHelp.Lib")

using namespace Olbbemi;

volatile long C_Dump::s_DumpCount = 0;

C_Dump::C_Dump()
{
	_invalid_parameter_handler lo_oldHandler, lo_newHandler;

	lo_newHandler = S_UnhandledHandler;
	lo_oldHandler = _set_invalid_parameter_handler(lo_newHandler);
	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);

	_set_purecall_handler(S_PureCallHandler);

	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	signal(SIGABRT, S_SignalHandler);
	signal(SIGINT, S_SignalHandler);
	signal(SIGILL, S_SignalHandler);
	signal(SIGFPE, S_SignalHandler);
	signal(SIGSEGV, S_SignalHandler);
	signal(SIGTERM, S_SignalHandler);

	S_SetHandlerDump();
}

void C_Dump::S_SetHandlerDump()
{
	SetUnhandledExceptionFilter(S_PrintException);
}

void C_Dump::S_UnhandledHandler(const wchar_t* pa_expression, const wchar_t* pa_function, const wchar_t* pa_filfile, unsigned int line, uintptr_t pa_Reserved)
{
	int *lo_crash_point = nullptr;
	*lo_crash_point = -1;
}

void C_Dump::S_MakeCrash()
{
	int *lo_crash_point = nullptr;
	*lo_crash_point = -1;
}

void C_Dump::S_SignalHandler(int pa_error)
{
	int *lo_crash_point = nullptr;
	*lo_crash_point = -1;
}

void C_Dump::S_PureCallHandler()
{
	int *lo_crash_point = nullptr;
	*lo_crash_point = -1;
}

LONG WINAPI C_Dump::S_PrintException(__in PEXCEPTION_POINTERS pa_ExceptionPointer)
{
	TCHAR lo_filename[255];
	SYSTEMTIME lo_system_curtime;

	GetLocalTime(&lo_system_curtime);
	long lo_DumpCount = InterlockedIncrement(&s_DumpCount);
	
	StringCchPrintf(lo_filename, 255, _TEXT("Dump_%d%02d%02d_%02d%02d%02d_%d_%d.dmp"), lo_system_curtime.wYear, lo_system_curtime.wMonth, lo_system_curtime.wDay, lo_system_curtime.wHour, lo_system_curtime.wMinute, lo_system_curtime.wSecond, lo_DumpCount, GetCurrentThreadId());

	HANDLE dumpfile = ::CreateFile(lo_filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dumpfile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION lo_minidumpExceptionInformation;

		lo_minidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
		lo_minidumpExceptionInformation.ExceptionPointers = pa_ExceptionPointer;
		lo_minidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpfile, MiniDumpWithFullMemory, &lo_minidumpExceptionInformation, NULL, NULL);
		CloseHandle(dumpfile);
	}
	else
	{
		TCHAR lo_action[] = _TEXT("DUMP"), lo_server[] = _TEXT("NONE");
		ST_Log lo_log({ "Createfile Error Code:" + GetLastError() });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}