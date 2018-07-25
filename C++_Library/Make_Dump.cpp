#define UNICODE
#define _UNICODE

#include "Make_Dump.h"

#include <DbgHelp.h>
#include <signal.h>
#include <Psapi.h>
#include <crtdbg.h>
#include <stdio.h>

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
	int iWorkIngMemory = 0;
	SYSTEMTIME stNowTime;

	long DumpCount = InterlockedIncrement(&_DumpCount);

	HANDLE hProcess = 0;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = GetCurrentProcess();

	if (hProcess == NULL)
		return 0;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		iWorkIngMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);

	CloseHandle(hProcess);
	
	WCHAR filename[255];
	GetLocalTime(&stNowTime);
	wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMP.dmp", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount, iWorkIngMemory);

	wprintf(L"Crash Error!!! %d.%d.%d / %d:%d:%d\n", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	wprintf(L"Save Dump File\n");


	HANDLE hDumpFile = ::CreateFile(filename, GENERIC_WRITE
		, FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInformation;

		minidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
		minidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		minidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory, &minidumpExceptionInformation,
			NULL, NULL);

		CloseHandle(hDumpFile);

	}
	else
	{
		wprintf(L"Error Code : %d\n",GetLastError());
	}

	return EXCEPTION_EXECUTE_HANDLER;
}