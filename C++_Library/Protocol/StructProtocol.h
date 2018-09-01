#ifndef StructProtocol_Info
#define StructProtocol_Info

#include <Windows.h>

#include <string>
using namespace std;

struct ST_Player
{
	LONG xpos, ypos;
	LONG64 account_no, session_id;
	string id, nickname;
};

#endif