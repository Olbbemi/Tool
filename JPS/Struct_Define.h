#ifndef Struct_Define
#define Struct_Define

enum class RECTANGLE_STATUS : int
{
	empty = 0,
	wall,
	head,
	tail
};

enum class KEY_STATUS : unsigned char
{
	paint_head = 10,
	paint_tail,
	paint_wall,
	paint_empty,
	paint_clear,
	start
};

struct NODE_INFO
{
	bool is_open_list, is_close_list;
	int startX, startY, endX, endY;
	RECTANGLE_STATUS status;
	int s_trace;
};

struct TRACE_INFO
{
	int col, row;
};

#define HEIGHT 20
#define WIDTH 50
#define PIXEL 20

#endif
