#ifndef StructProtocol_Info
#define StructProtocol_Info

#include <Windows.h>

#include <string>
using namespace std;

struct ST_PLAYER
{
	WORD xpos, ypos;
	LONG pre_width_index, pre_height_index;
	LONG cur_width_index, cur_height_index;
	LONG64 account_no, session_id;
	wstring id, nickname;

	ST_PLAYER()
	{
		pre_width_index = -1;	pre_height_index = -1;
		cur_width_index = -1;	cur_height_index = -1;
	}
};

#endif