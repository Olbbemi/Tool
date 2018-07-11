#include "Precompile.h"
#include "Screen.h"
#include "Astar.h"

SCREEN::SCREEN() : cnt(0)
{
	lock = false;
	Astar = new ASTAR();

	line_pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 255));

	head_brush = CreateSolidBrush(RGB(255, 0, 0));
	tail_brush = CreateSolidBrush(RGB(0, 0, 255));
	wall_brush = CreateSolidBrush(RGB(0, 0, 0));
	empty_brush = CreateSolidBrush(RGB(255, 255, 255));
	open_brush = CreateSolidBrush(RGB(55, 100, 50));
	close_brush = CreateSolidBrush(RGB(125, 155, 125));

	local_key = (BYTE)KEY_STATUS::paint_wall;
	shape.resize(HEIGHT);

	for (int i = 0; i < HEIGHT; i++)
		shape[i].resize(WIDTH);

	int pivotY = 0;
	for (int i = 0; i < HEIGHT; i++)
	{
		int pivotX = 0;
		pivotY += PIXEL;

		for (int j = 0; j < WIDTH; j++)
		{
			pivotX += PIXEL;

			shape[i][j].startX = pivotX;
			shape[i][j].startY = pivotY;
			shape[i][j].endX = shape[i][j].startX + PIXEL;
			shape[i][j].endY = shape[i][j].startY + PIXEL;
			shape[i][j].status = RECTANGLE_STATUS::empty;
		}
	}
}

SCREEN::~SCREEN()	{}

void SCREEN::SetBufferInfo(HDC SDC, RECT Srect)
{
	screen_DC = SDC;
	memory_DC = CreateCompatibleDC(SDC);
	memory_rect = Srect;
	memory_bitmap = CreateCompatibleBitmap(SDC, Srect.right, Srect.bottom);
	SelectObject(memory_DC, (HBITMAP)memory_bitmap);
}

void SCREEN::ChangeKey(BYTE key)
{
	if (lock == true)
		return;

	local_key = key;
}

void SCREEN::SetPosition(int xPos, int yPos)
{
	xpos = xPos;
	ypos = yPos;
}

void SCREEN::Run()
{
	Update();
	Render();
}

void SCREEN::Update()
{
	pair<int,int> temp;
	bool check;
	int sX, eX, sY, eY;

	switch (local_key)
	{
		case (BYTE)KEY_STATUS::paint_head:
			if (xpos != -1 && ypos != -1)
			{
				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (shape[i][j].status == RECTANGLE_STATUS::head)
						{
							shape[i][j].status = RECTANGLE_STATUS::empty;
							check = true;
							break;
						}
					}
					
					if (check == true)
						break;
				}

				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (shape[i][j].startX <= xpos && xpos <= shape[i][j].endX && shape[i][j].startY <= ypos && ypos <= shape[i][j].endY)
						{
							shape[i][j].status = RECTANGLE_STATUS::head;
							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				xpos = ypos = -1;
			}
			break;

		case (BYTE)KEY_STATUS::paint_tail:
			if (xpos != -1 && ypos != -1)
			{
				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (shape[i][j].status == RECTANGLE_STATUS::tail)
						{
							shape[i][j].status = RECTANGLE_STATUS::empty;
							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (shape[i][j].startX <= xpos && xpos <= shape[i][j].endX && shape[i][j].startY <= ypos && ypos <= shape[i][j].endY)
						{
							shape[i][j].status = RECTANGLE_STATUS::tail;
							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				xpos = ypos = -1;
			}
			break;

		case (BYTE)KEY_STATUS::paint_wall:
			if (xpos != -1 && ypos != -1)
			{
				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (shape[i][j].startX <= xpos && xpos <= shape[i][j].endX && shape[i][j].startY <= ypos && ypos <= shape[i][j].endY)
						{
							if (shape[i][j].status == RECTANGLE_STATUS::head || shape[i][j].status == RECTANGLE_STATUS::tail)
								continue;
							else if (shape[i][j].status == RECTANGLE_STATUS::empty)
								shape[i][j].status = RECTANGLE_STATUS::wall;

							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				xpos = ypos = -1;
			}
			break;

			case (BYTE)KEY_STATUS::paint_empty:
				if (xpos != -1 && ypos != -1)
				{
					for (int i = 0; i < HEIGHT; i++)
					{
						bool check = false;

						for (int j = 0; j < WIDTH; j++)
						{
							if (shape[i][j].startX <= xpos && xpos <= shape[i][j].endX && shape[i][j].startY <= ypos && ypos <= shape[i][j].endY)
							{
								if (shape[i][j].status == RECTANGLE_STATUS::head || shape[i][j].status == RECTANGLE_STATUS::tail)
									continue;
								else if (shape[i][j].status == RECTANGLE_STATUS::wall)
									shape[i][j].status = RECTANGLE_STATUS::empty;

								check = true;
								break;
							}
						}

						if (check == true)
							break;
					}

					xpos = ypos = -1;
				}
				break;

		case (BYTE)KEY_STATUS::paint_clear:
			for (int i = 0; i < HEIGHT; i++)
				for (int j = 0; j < WIDTH; j++)
					shape[i][j].status = RECTANGLE_STATUS::empty;
			break;

		case (BYTE)KEY_STATUS::start:

			if (cnt == 0)
			{
				lock = true;
				for (int i = 0; i < HEIGHT; i++)
				{
					for (int j = 0; j < WIDTH; j++)
					{
						if (shape[i][j].status == RECTANGLE_STATUS::head)
						{
							sX = j;
							sY = i;
						}
						else if (shape[i][j].status == RECTANGLE_STATUS::tail)
						{
							eX = j;
							eY = i;
						}
					}
				}
				
				result.clear();
				Astar->SetPosition(sX, sY, eX, eY);
				cnt++;
			}
			
			check = Astar->Astar(shape, result);

			if (check == true)
			{
				cnt = 0;
				local_key = (BYTE)KEY_STATUS::paint_wall;
				lock = false;
			}
				
			break;
	}
}

void SCREEN::Render()
{
	PatBlt(memory_DC, memory_rect.left, memory_rect.top, memory_rect.right, memory_rect.bottom, PATCOPY);

	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			if (shape[i][j].status == RECTANGLE_STATUS::empty)
				SelectObject(memory_DC, (HBRUSH)empty_brush);
			else if (shape[i][j].status == RECTANGLE_STATUS::head)
				SelectObject(memory_DC, (HBRUSH)head_brush);
			else if (shape[i][j].status == RECTANGLE_STATUS::tail)
				SelectObject(memory_DC, (HBRUSH)tail_brush);
			else if (shape[i][j].status == RECTANGLE_STATUS::wall)
				SelectObject(memory_DC, (HBRUSH)wall_brush);
			else if (shape[i][j].status == RECTANGLE_STATUS::open_list)
				SelectObject(memory_DC, (HBRUSH)open_brush);
			else if (shape[i][j].status == RECTANGLE_STATUS::close_list)
				SelectObject(memory_DC, (HBRUSH)close_brush);

			Rectangle(memory_DC, shape[i][j].startX, shape[i][j].startY, shape[i][j].endX, shape[i][j].endY);
		}
	}

	old_pen = (HPEN)SelectObject(memory_DC, (HPEN)line_pen);
	for (auto p = result.begin(); p != result.end(); p++)
	{
		MoveToEx(memory_DC, (*p)->curXpos, (*p)->curYpos, NULL);
		LineTo(memory_DC, (*p)->preXpos, (*p)->preYpos);
	}
	SelectObject(memory_DC, (HPEN)old_pen);

	BitBlt(screen_DC, memory_rect.left, memory_rect.top, memory_rect.right, memory_rect.bottom,memory_DC, memory_rect.left, memory_rect.top, SRCCOPY);
}