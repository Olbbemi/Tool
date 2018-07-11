#ifndef Struct_Define
#define Struct_Define

enum class RECTANGLE_STATUS : unsigned char
{
	empty = 0,
	wall,
	head,
	tail,
	open_list,
	close_list
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

struct INFO
{
	int startX, startY, endX, endY;
	RECTANGLE_STATUS status;
};

struct TRACE
{
	int curXpos, curYpos, preXpos, preYpos;
};


#define HEIGHT 23
#define WIDTH 35
#define PIXEL 30

#endif
