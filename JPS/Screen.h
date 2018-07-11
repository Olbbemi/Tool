#ifndef Screen_Info
#define Screen_Info

#include "Struct_Define.h"

#include <Windows.h>
#include <vector>
#include <list>
using namespace std;

class JUMPPOINTSEARCH;


class SCREEN
{
private:
	HDC m_screen_DC, m_memory_DC;
	HBITMAP m_memory_bitmap;
	RECT m_memory_rect;

	HPEN m_old_pen, m_line_pen;
	HBRUSH m_head_brush, m_tail_brush, m_wall_brush, m_empty_brush, m_open_brush, m_close_brush, m_circle_brush;

	HBRUSH m_trace[50];
	int m_count;

	vector< vector<NODE_INFO> > m_shape;
	JUMPPOINTSEARCH *m_jump_point_search;

	bool m_lock;
	BYTE m_local_key;
	int m_xpos, m_ypos, m_cnt;

	list<TRACE_INFO*> m_bre_result;
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