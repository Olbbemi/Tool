#include "Profile.h"

PROFILE::NODE::NODE(const char *str, _int64 val)
{
	MinCnt = MaxCnt = 0;
	cnt = total = 0;
	startTime = val;
	strcpy_s(name, sizeof(name), str);

	for (int i = 0; i < 3; i++)
	{
		Max[i] = 0;
		Min[i] = 0x7FFFFFFFFFFFFFFF;
	}
}

PROFILE::PROFILE() : buf("Profile_")
{
	QueryPerformanceFrequency(&frequency);
}

void PROFILE::GetTime()
{
	char date[20], tail[5] = ".txt";
	time_t timer = time(NULL);
	struct tm data;

	localtime_s(&data, &timer);

	sprintf_s(date, "%4d_%02d%02d_%02d%02d%02d", data.tm_year + 1900, data.tm_mon + 1, data.tm_mday, data.tm_hour, data.tm_min, data.tm_sec);
	strcat_s(buf, sizeof(buf), date);
	strcat_s(buf, sizeof(buf), tail);
}

void Profile_Begin(PROFILE *obj, const char *str, int line)
{
	int flag = (unsigned int)PROFILE::STATUS::Normal;
	for (auto p = obj->L.begin(); p != obj->L.end(); p++)
	{
		if (!strcmp((*p)->name, str))
		{
			if ((*p)->startTime != 0)
				flag = (unsigned int)PROFILE::STATUS::Fail;
			else
			{ 
				flag = (unsigned int)PROFILE::STATUS::Success;
				QueryPerformanceCounter(&obj->cnt1);
				(*p)->startTime = obj->cnt1.QuadPart;
			}
				
			break;
		}
	}
		
	if (flag == (unsigned int)PROFILE::STATUS::Fail)
	{
		printf("%d : Need to End Function", line);
		throw;
	}
	else if (flag == (unsigned int)PROFILE::STATUS::Normal)
	{
		QueryPerformanceCounter(&obj->cnt1);
		PROFILE::NODE *temp = new PROFILE::NODE(str, obj->cnt1.QuadPart);
		obj->L.push_back(temp);
	}
}

void Profile_End(PROFILE *obj, const char *str)
{
	QueryPerformanceCounter(&obj->cnt2);
	for (auto p = obj->L.begin(); p != obj->L.end(); p++)
	{
		if (!strcmp((*p)->name, str))
		{
			(*p)->cnt++;
			(*p)->total += obj->cnt2.QuadPart - (*p)->startTime;

			if ((*p)->MinCnt != 3 && (*p)->Min[(*p)->MinCnt] > obj->cnt2.QuadPart - (*p)->startTime)	
				(*p)->Min[(*p)->MinCnt++] = obj->cnt2.QuadPart - (*p)->startTime;
			else if((*p)->MinCnt == 3)
			{
				int index = 0;
				_int64 val = (*p)->Min[0];
				for (int i = 1; i < 3; i++)
				{
					if (val < (*p)->Min[i])
					{
						val = (*p)->Min[i];
						index = i;
					}
				}

				if (obj->cnt2.QuadPart - (*p)->startTime < val)
					(*p)->Min[index] = obj->cnt2.QuadPart - (*p)->startTime;
			}

			if ((*p)->MaxCnt != 3 && (*p)->Max[(*p)->MaxCnt] < obj->cnt2.QuadPart - (*p)->startTime)
				(*p)->Max[(*p)->MaxCnt++] = obj->cnt2.QuadPart - (*p)->startTime;
			else if ((*p)->MaxCnt == 3)
			{
				int index = 0;
				_int64 val = (*p)->Max[0];
				for (int i = 1; i < 3; i++)
				{
					if (val > (*p)->Max[i])
					{
						val = (*p)->Max[i];
						index = i;
					}
				}

				if (obj->cnt2.QuadPart - (*p)->startTime > val)
					(*p)->Max[index] = obj->cnt2.QuadPart - (*p)->startTime;
			}

			(*p)->startTime = 0;
			break;
		}
	}
}

void PROFILE::Save()
{
	if (GetAsyncKeyState(VK_HOME) & 0x8001)
	{
		PROFILE::GetTime();

		fopen_s(&output, buf, "w");
		fprintf_s(output, " ----------------------------------------------------------------------------------------------------\n");
		fprintf_s(output, "|     FileName     |       Average       |      Min_Value      |      Max_Value      |     Count     |\n");

		for (auto p = L.begin(); p != L.end(); p++)
		{
			_int64 MaxValue = (*p)->Max[0], MinValue = (*p)->Min[0];

			for (int i = 1; i < (*p)->MaxCnt; i++)
			{
				if ((*p)->Max[i] > MaxValue)
				{
					(*p)->total -= MaxValue;
					MaxValue = (*p)->Max[i];
				}
				else
					(*p)->total -= (*p)->Max[i];
			}
			(*p)->cnt -= ((*p)->MaxCnt - 1);

			for (int i = 1; i < (*p)->MinCnt; i++)
			{
				if ((*p)->Min[i] < MinValue)
				{
					(*p)->total -= MinValue;
					MinValue = (*p)->Min[i];
				}
				else
					(*p)->total -= (*p)->Min[i];
			}
			(*p)->cnt -= ((*p)->MinCnt - 1);

			fprintf_s(output, "|%18s|%21.4Lf|%21.4Lf|%21.4Lf|%15lld|\n", (*p)->name, ((long double)(*p)->total / (long double)(*p)->cnt / (long double)frequency.QuadPart * MICRO), ((long double)MinValue / (long double)frequency.QuadPart * MICRO), ((long double)MaxValue / (long double)frequency.QuadPart * MICRO), (*p)->cnt);
		}
			
		fprintf_s(output, " ----------------------------------------------------------------------------------------------------\n");
		fclose(output);

		buf[0] = '\0';
	}
}

void PROFILE::Delete()
{
	if (GetAsyncKeyState(VK_END) & 0x8001)	
		L.clear();
}
