#include "Precompile.h"
#include "LanServer.h"
#include "Contents.h"

#include "Log/Log.h"
#include "Profile/Profile.h"

using namespace Olbbemi;

#define WorkerThreadCount 4

C_Log g_log;
C_Profile g_profile(4);
bool g_is_start = true, g_is_stop = false;
ULONGLONG g_pre_time = GetTickCount64();

int main()
{
	TCHAR lo_ip[] = _TEXT("0.0.0.0");
	WORD  lo_port = 6000;

	C_LanServer* lo_server = new C_Contents;

	_MAKEDIR("Server");

	while (1)
	{
		ULONGLONG lo_cur_time = GetTickCount64();

		if (lo_cur_time - g_pre_time >= 1500)
		{
			if (g_is_start == false)
			{
				printf("-----------------------------------------------------\n");
				printf("LFStack Alloc: %d, LFStack Use: %d\n\n", lo_server->M_StackAllocCount(), lo_server->M_StackUseCount());
				printf("LFQueue TotalAlloc: %d, LFQueue TotalUse: %d\n\n", lo_server->M_QueueAllocCount(), lo_server->M_QueueUseCount());
				printf("TLSPool Alloc: %d, TLSPool Chunk: %d, TLSPool Node: %d\n\n", lo_server->M_TLSPoolAllocCount(), lo_server->M_TLSPoolChunkCount(), lo_server->M_TLSPoolNodeCount());
				printf("-----------------------------------------------------\n");
			}

			g_pre_time = lo_cur_time;
		}

		if (GetAsyncKeyState(VK_F1) & 0x0001)
		{
			if (g_is_start == true)
			{
				lo_server->M_Start(true, 4, lo_ip, lo_port, 100);
				printf("Server On!!!\n");
			}

			g_is_start = false;		g_is_stop = true;
		}

		if (GetAsyncKeyState(VK_F2) & 0x0001)
		{
			if (g_is_stop == true)
			{
				lo_server->M_Stop();
				printf("Server Off!!!\n");
			}

			g_is_start = true;	g_is_stop = false;
		}

		g_profile.M_Save();
		Sleep(1);
	}
}