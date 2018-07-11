#include "Precompile.h"
#include "Screen.h"
#include "Jump_Point_Search.h"

#define MAXVALUE 500

SCREEN::SCREEN() : m_cnt(0), m_count(1000)
{
	m_lock = false;
	m_jump_point_search = new JUMPPOINTSEARCH(MAXVALUE);

	m_line_pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 255));

	m_head_brush = CreateSolidBrush(RGB(255, 0, 0));
	m_tail_brush = CreateSolidBrush(RGB(0, 0, 255));
	m_wall_brush = CreateSolidBrush(RGB(0, 0, 0));
	m_empty_brush = CreateSolidBrush(RGB(255, 255, 255));
	m_open_brush = CreateSolidBrush(RGB(55, 100, 50));
	m_close_brush = CreateSolidBrush(RGB(0, 155, 125));

	m_circle_brush = CreateSolidBrush(RGB(50, 50, 50));


	for (int i = 0; i < 50; i++)
		m_trace[i] = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));

	m_local_key = (BYTE)KEY_STATUS::paint_wall;
	m_shape.resize(HEIGHT);

	for (int i = 0; i < HEIGHT; i++)
		m_shape[i].resize(WIDTH);

	int pivotY = 0;
	for (int i = 0; i < HEIGHT; i++)
	{
		int pivotX = 0;
		pivotY += PIXEL;

		for (int j = 0; j < WIDTH; j++)
		{
			pivotX += PIXEL;

			m_shape[i][j].startX = pivotX;
			m_shape[i][j].startY = pivotY;
			m_shape[i][j].endX = m_shape[i][j].startX + PIXEL;
			m_shape[i][j].endY = m_shape[i][j].startY + PIXEL;
			m_shape[i][j].status = RECTANGLE_STATUS::empty;
		}
	}
}

SCREEN::~SCREEN()	{}

void SCREEN::SetBufferInfo(HDC SDC, RECT Srect)
{
	m_screen_DC = SDC;
	m_memory_DC = CreateCompatibleDC(SDC);
	m_memory_rect = Srect;
	m_memory_bitmap = CreateCompatibleBitmap(SDC, Srect.right, Srect.bottom);
	SelectObject(m_memory_DC, (HBITMAP)m_memory_bitmap);
}

void SCREEN::ChangeKey(BYTE key)
{
	if (m_lock == true)
		return;

	m_local_key = key;
}

void SCREEN::SetPosition(int xPos, int yPos)
{
	m_xpos = xPos;
	m_ypos = yPos;
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

	switch (m_local_key)
	{
		case (BYTE)KEY_STATUS::paint_head:
			if (m_xpos != -1 && m_ypos != -1)
			{
				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (m_shape[i][j].status == RECTANGLE_STATUS::head)
						{
							m_shape[i][j].status = RECTANGLE_STATUS::empty;
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
						if (m_shape[i][j].startX <= m_xpos && m_xpos <= m_shape[i][j].endX && m_shape[i][j].startY <= m_ypos && m_ypos <= m_shape[i][j].endY)
						{
							m_shape[i][j].status = RECTANGLE_STATUS::head;
							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				m_xpos = m_ypos = -1;
			}
			break;

		case (BYTE)KEY_STATUS::paint_tail:
			if (m_xpos != -1 && m_ypos != -1)
			{
				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (m_shape[i][j].status == RECTANGLE_STATUS::tail)
						{
							m_shape[i][j].status = RECTANGLE_STATUS::empty;
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
						if (m_shape[i][j].startX <= m_xpos && m_xpos <= m_shape[i][j].endX && m_shape[i][j].startY <= m_ypos && m_ypos <= m_shape[i][j].endY)
						{
							m_shape[i][j].status = RECTANGLE_STATUS::tail;
							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				m_xpos = m_ypos = -1;
			}
			break;

		case (BYTE)KEY_STATUS::paint_wall:
			if (m_xpos != -1 && m_ypos != -1)
			{
				for (int i = 0; i < HEIGHT; i++)
				{
					bool check = false;

					for (int j = 0; j < WIDTH; j++)
					{
						if (m_shape[i][j].startX <= m_xpos && m_xpos <= m_shape[i][j].endX && m_shape[i][j].startY <= m_ypos && m_ypos <= m_shape[i][j].endY)
						{
							if (m_shape[i][j].status == RECTANGLE_STATUS::head || m_shape[i][j].status == RECTANGLE_STATUS::tail)
								continue;
							else if (m_shape[i][j].status == RECTANGLE_STATUS::empty)
								m_shape[i][j].status = RECTANGLE_STATUS::wall;

							check = true;
							break;
						}
					}

					if (check == true)
						break;
				}

				m_xpos = m_ypos = -1;
			}
			break;

			case (BYTE)KEY_STATUS::paint_empty:
				if (m_xpos != -1 && m_ypos != -1)
				{
					for (int i = 0; i < HEIGHT; i++)
					{
						bool check = false;

						for (int j = 0; j < WIDTH; j++)
						{
							if (m_shape[i][j].startX <= m_xpos && m_xpos <= m_shape[i][j].endX && m_shape[i][j].startY <= m_ypos && m_ypos <= m_shape[i][j].endY)
							{
								if (m_shape[i][j].status == RECTANGLE_STATUS::head || m_shape[i][j].status == RECTANGLE_STATUS::tail)
									continue;
								else if (m_shape[i][j].status == RECTANGLE_STATUS::wall)
									m_shape[i][j].status = RECTANGLE_STATUS::empty;

								check = true;
								break;
							}
						}

						if (check == true)
							break;
					}

					m_xpos = m_ypos = -1;
				}
				break;

		case (BYTE)KEY_STATUS::paint_clear:
			for (int i = 0; i < HEIGHT; i++)
				for (int j = 0; j < WIDTH; j++)
					m_shape[i][j].status = RECTANGLE_STATUS::empty;
			break;

		case (BYTE)KEY_STATUS::start:

			if (m_cnt == 0)
			{
				m_lock = true;
				for (int i = 0; i < HEIGHT; i++)
				{
					for (int j = 0; j < WIDTH; j++)
					{
						if (m_shape[i][j].status == RECTANGLE_STATUS::head)
						{
							sX = j;
							sY = i;
						}
						else if (m_shape[i][j].status == RECTANGLE_STATUS::tail)
						{
							eX = j;
							eY = i;
						}
					}
				}
				
				m_jump_point_search->SetPosition(m_shape, sX, sY, eX, eY);
				m_cnt++;
			}
			
			check = m_jump_point_search->JPS(m_shape, m_bre_result, m_count++);

			if (check == true)
			{
				m_cnt = 0;
				m_local_key = (BYTE)KEY_STATUS::paint_wall;
				m_lock = false;
			}
				
			break;
	}
}

void SCREEN::Render()
{
	PatBlt(m_memory_DC, m_memory_rect.left, m_memory_rect.top, m_memory_rect.right, m_memory_rect.bottom, PATCOPY);

	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			if (m_shape[i][j].status == RECTANGLE_STATUS::empty)
				SelectObject(m_memory_DC, (HBRUSH)m_empty_brush);
			else if (m_shape[i][j].status == RECTANGLE_STATUS::head)
				SelectObject(m_memory_DC, (HBRUSH)m_head_brush);
			else if (m_shape[i][j].status == RECTANGLE_STATUS::tail)
				SelectObject(m_memory_DC, (HBRUSH)m_tail_brush);
			else if (m_shape[i][j].status == RECTANGLE_STATUS::wall)
				SelectObject(m_memory_DC, (HBRUSH)m_wall_brush);

			if (m_shape[i][j].is_open_list == true)
				SelectObject(m_memory_DC, (HBRUSH)m_open_brush);
			
			if (m_shape[i][j].is_close_list == true)
				SelectObject(m_memory_DC, (HBRUSH)m_close_brush);


			for (int k = 1000; k < 1050; k++)
				if (m_shape[i][j].s_trace == k)
					SelectObject(m_memory_DC, (HBRUSH)m_trace[k - 1000]);
				
			Rectangle(m_memory_DC, m_shape[i][j].startX, m_shape[i][j].startY, m_shape[i][j].endX, m_shape[i][j].endY);
		}
	}

	m_old_pen = (HPEN)SelectObject(m_memory_DC, (HPEN)m_line_pen);
	SelectObject(m_memory_DC, (HPEN)m_circle_brush);
	for (auto p = m_bre_result.begin(); p != m_bre_result.end(); p++)
		Ellipse(m_memory_DC, m_shape[(*p)->row][(*p)->col].startX, m_shape[(*p)->row][(*p)->col].startY, m_shape[(*p)->row][(*p)->col].endX, m_shape[(*p)->row][(*p)->col].endY);

	SelectObject(m_memory_DC, (HPEN)m_old_pen);
	SelectObject(m_memory_DC, (HBRUSH)GetStockObject(WHITE_BRUSH));

	BitBlt(m_screen_DC, m_memory_rect.left, m_memory_rect.top, m_memory_rect.right, m_memory_rect.bottom, m_memory_DC, m_memory_rect.left, m_memory_rect.top, SRCCOPY);
}