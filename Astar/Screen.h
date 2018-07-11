#ifndef Screen_Info
#define Screen_Info

#include "Struct_Define.h"

#include <Windows.h>
#include <vector>
#include <deque>
using namespace std;

class ASTAR;

class SCREEN
{
private:
	HDC screen_DC, memory_DC;
	HBITMAP memory_bitmap;
	RECT memory_rect;

	HPEN old_pen, line_pen;
	HBRUSH head_brush, tail_brush, wall_brush, empty_brush, open_brush, close_brush;

	vector< vector<INFO> > shape;
	ASTAR *Astar;

	bool lock;
	BYTE local_key;
	int xpos, ypos, cnt;

	deque<TRACE*> result;
public:
	SCREEN();
	~SCREEN();

	void SetBufferInfo(HDC screen_DC, RECT screen_rect);
	void ChangeKey(BYTE key);
	void SetPosition(int xPos, int yPos);
	void Run();
	void Update();
	void Render();
};

#endif