#ifndef Profile
#define Profile

#define _UNICODE
#define UNICODE

#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <tchar.h>
#include <list>
#include <algorithm>

using namespace std;

#define MICRO 1000000.0
#define Profiler
#ifdef Profiler
	#define BEGIN(Obj, Name, LINE)	Profile_Begin(Obj, Name, LINE)
	#define END(Obj, Name)			Profile_End(Obj, Name)
#else
	#define BEGIN(None)
	#define END(None)
#endif

class PROFILE
{
private:
	class NODE
	{
		TCHAR name[25];
		int MinCnt, MaxCnt;

		_int64 Min[3], Max[3], cnt, startTime, total;

	public:
		NODE(const PTCHAR str, _int64 val);

		friend class PROFILE;
		friend void Profile_Begin(PROFILE *obj, const PTCHAR str, int line);
		friend void Profile_End(PROFILE *obj, const PTCHAR str);
	};

	enum class STATUS
	{
		Normal = 0,
		Fail,
		Success
	};

	TCHAR buf[30];
	LARGE_INTEGER frequency, cnt1, cnt2;
	FILE *output;

	void GetTime();

public:
	list<NODE*> L;

	PROFILE();
	void Save();
	void Delete();

	friend void Profile_Begin(PROFILE *obj, const PTCHAR str, int line);
	friend void Profile_End(PROFILE *obj, const PTCHAR str);
};

void Profile_Begin(PROFILE *obj, const PTCHAR str, int line);
void Profile_End(PROFILE *obj, const PTCHAR str);

#endif
