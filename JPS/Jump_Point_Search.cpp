#include "Precompile.h"
#include "Jump_Point_Search.h"
#include "Bresenham.h"

#include <algorithm>

bool cmp(JUMPPOINTSEARCH::NODE* &u, JUMPPOINTSEARCH::NODE* &v);

JUMPPOINTSEARCH::JUMPPOINTSEARCH(int p_search_max)
{
	m_bresenham = new BRESENHAM;
	m_max_value = p_search_max;
}

void JUMPPOINTSEARCH::SetPosition(vector< vector<NODE_INFO> > &p_tile, int p_SxPos, int p_SyPos, int p_ExPos, int p_EyPos)
{
	m_Open_List.clear();
	m_Close_List.clear();

	m_endX = p_ExPos;	m_endY = p_EyPos;

	NODE *new_node = new NODE;
	new_node->s_parent = nullptr;
	new_node->s_row = p_SyPos;
	new_node->s_col = p_SxPos;
	new_node->s_G = 0.0f;
	new_node->s_H = ((float)abs(m_endX - p_SxPos) + (float)abs(m_endY - p_SyPos));
	new_node->s_F = new_node->s_G + new_node->s_H;
	new_node->s_direction = -1;

	m_Open_List.push_back(new_node);
	p_tile[new_node->s_row][new_node->s_col].is_open_list = true;
}

void JUMPPOINTSEARCH::MakeNode(vector< vector<NODE_INFO> > &p_tile, char p_direction, int p_xPos, int p_yPos, float p_G, NODE *ancestor)
{
	bool check = false;
	for (auto p = m_Open_List.begin(); p != m_Open_List.end(); ++p)
	{
		if ((*p)->s_row == p_yPos && (*p)->s_col == p_xPos && ancestor->s_G + p_G < (*p)->s_G)
		{
			(*p)->s_G = ancestor->s_G + p_G;
			(*p)->s_F = (*p)->s_G + (*p)->s_H;
			(*p)->s_parent = ancestor;
			check = true;
			break;
		}
	}

	if (check == true)
		return;

	for (auto p = m_Close_List.begin(); p != m_Close_List.end(); ++p)
	{
		if ((*p)->s_row == p_yPos && (*p)->s_col == p_xPos && ancestor->s_G + p_G < (*p)->s_G)
		{
			(*p)->s_G = ancestor->s_G + p_G;
			(*p)->s_F = (*p)->s_G + (*p)->s_H;
			(*p)->s_parent = ancestor;
			check = true;
			break;
		}
	}

	if (check == true)
		return;

	NODE *new_node = new NODE;
	new_node->s_parent = ancestor;
	new_node->s_row = p_yPos;
	new_node->s_col = p_xPos;
	new_node->s_G = ancestor->s_G + p_G;
	new_node->s_H = ((float)abs(m_endX - p_xPos) + (float)abs(m_endY - p_yPos));
	new_node->s_F = new_node->s_G + new_node->s_H;
	new_node->s_direction = p_direction;

	m_Open_List.push_back(new_node);
}

bool inline JUMPPOINTSEARCH::CheckPath(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos)
{
	if (p_tile[p_yPos][p_xPos].status == RECTANGLE_STATUS::wall || p_tile[p_yPos][p_xPos].status == RECTANGLE_STATUS::head)
		return false;

	return true;
}

list<TRACE_INFO*> JUMPPOINTSEARCH::MakeTrace(vector< vector<NODE_INFO> > &p_tile, NODE *p_pivot)
{
	list<TRACE_INFO*> list;

	NODE *pre = p_pivot->s_parent, *cur = p_pivot->s_parent->s_parent;

	TRACE_INFO *start_spot = new TRACE_INFO;
	start_spot->col = p_pivot->s_col;
	start_spot->row = p_pivot->s_row;
	list.push_back(start_spot);

	while (cur != nullptr)
	{
		if (m_bresenham->Bresenham(p_tile, p_pivot->s_col, p_pivot->s_row, cur->s_col, cur->s_row) == true)
		{
			pre = cur;
			cur = cur->s_parent;
		}
		else
		{
			TRACE_INFO *node = new TRACE_INFO;

			node->col = pre->s_col;
			node->row = pre->s_row;
			list.push_back(node);
			p_pivot = pre;
		}
	}

	TRACE_INFO *end_spot = new TRACE_INFO;
	end_spot->col = pre->s_col;
	end_spot->row = pre->s_row;
	list.push_back(end_spot);

	return list;
}

bool JUMPPOINTSEARCH::JPS(vector< vector<NODE_INFO> > &p_tile, list<TRACE_INFO*> &p_trace, int p_color)
{
	if (m_Open_List.size() == 0) // open_list 존재하지 않으면 더이상 경로 없음을 의미	
		return true;

	NODE *pivot = m_Open_List.front();	m_Open_List.pop_front();
	m_Close_List.push_back(pivot);

	p_tile[pivot->s_row][pivot->s_col].is_close_list = true;

	if (pivot->s_row == m_endY && pivot->s_col == m_endX)
	{
		p_trace = MakeTrace(p_tile, pivot);
		return true;
	}

	if (pivot->s_parent == nullptr)
	{
		UUSearch(p_tile, pivot->s_col, pivot->s_row - 1, pivot);
		URSearch(p_tile, pivot->s_col + 1, pivot->s_row - 1, pivot);
		RRSearch(p_tile, pivot->s_col + 1, pivot->s_row, pivot);
		DRSearch(p_tile, pivot->s_col + 1, pivot->s_row + 1, pivot);
		DDSearch(p_tile, pivot->s_col, pivot->s_row + 1, pivot);
		DLSearch(p_tile, pivot->s_col - 1, pivot->s_row + 1, pivot);
		LLSearch(p_tile, pivot->s_col - 1, pivot->s_row, pivot);
		ULSearch(p_tile, pivot->s_col - 1, pivot->s_row - 1, pivot);
	}
	else
	{
		switch (pivot->s_direction)
		{
			case (char)DIRECTION::s_UU:
				UUSearch(p_tile, pivot->s_col, pivot->s_row - 1, pivot);

				if(0 <= pivot->s_row - 1 && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row][pivot->s_col - 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					ULSearch(p_tile, pivot->s_col - 1, pivot->s_row - 1, pivot);
				
				if (0 <= pivot->s_row - 1 && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row][pivot->s_col + 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					URSearch(p_tile, pivot->s_col + 1, pivot->s_row - 1, pivot);
				break;

			case (char)DIRECTION::s_UR:
				UUSearch(p_tile, pivot->s_col, pivot->s_row - 1, pivot);
				URSearch(p_tile, pivot->s_col + 1, pivot->s_row - 1, pivot);
				RRSearch(p_tile, pivot->s_col + 1, pivot->s_row, pivot);

				if (0 <= pivot->s_row - 1 && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row][pivot->s_col - 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					ULSearch(p_tile, pivot->s_col - 1, pivot->s_row - 1, pivot);

				if (pivot->s_row + 1 < HEIGHT && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row + 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					DRSearch(p_tile, pivot->s_col + 1, pivot->s_row + 1, pivot);
				break;

			case (char)DIRECTION::s_RR:
				RRSearch(p_tile, pivot->s_col + 1, pivot->s_row, pivot);

				if (0 <= pivot->s_row - 1 && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row - 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					URSearch(p_tile, pivot->s_col + 1, pivot->s_row - 1, pivot);
				
				if (pivot->s_row + 1 < HEIGHT && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row + 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					DRSearch(p_tile, pivot->s_col + 1, pivot->s_row + 1, pivot);
				break;

			case (char)DIRECTION::s_DR:
				RRSearch(p_tile, pivot->s_col + 1, pivot->s_row, pivot);
				DRSearch(p_tile, pivot->s_col + 1, pivot->s_row + 1, pivot);
				DDSearch(p_tile, pivot->s_col, pivot->s_row + 1, pivot);

				if (0 <= pivot->s_row - 1 && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row - 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					URSearch(p_tile, pivot->s_col + 1, pivot->s_row - 1, pivot);

				if (pivot->s_row + 1 < HEIGHT && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row][pivot->s_col - 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					DLSearch(p_tile, pivot->s_col - 1, pivot->s_row + 1, pivot);

				break;

			case (char)DIRECTION::s_DD:
				DDSearch(p_tile, pivot->s_col, pivot->s_row + 1, pivot);

				if (pivot->s_row + 1 < HEIGHT && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row][pivot->s_col - 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					DLSearch(p_tile, pivot->s_col - 1, pivot->s_row + 1, pivot);

				if (pivot->s_row + 1 < HEIGHT && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row][pivot->s_col + 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					DRSearch(p_tile, pivot->s_col + 1, pivot->s_row + 1, pivot);
				break;

			case (char)DIRECTION::s_DL:
				LLSearch(p_tile, pivot->s_col - 1, pivot->s_row, pivot);
				DLSearch(p_tile, pivot->s_col - 1, pivot->s_row + 1, pivot);
				DDSearch(p_tile, pivot->s_col, pivot->s_row + 1, pivot);

				if (0 <= pivot->s_row - 1 && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row - 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					ULSearch(p_tile, pivot->s_col - 1, pivot->s_row - 1, pivot);

				if (pivot->s_row + 1 < HEIGHT && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row][pivot->s_col + 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					DRSearch(p_tile, pivot->s_col + 1, pivot->s_row + 1, pivot);
				break;

			case (char)DIRECTION::s_LL:
				LLSearch(p_tile, pivot->s_col - 1, pivot->s_row, pivot);

				if (0 <= pivot->s_row - 1 && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row - 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					ULSearch(p_tile, pivot->s_col - 1, pivot->s_row - 1, pivot);

				if (pivot->s_row + 1 < HEIGHT && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row + 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					DLSearch(p_tile, pivot->s_col - 1, pivot->s_row + 1, pivot);
				break;

			case (char)DIRECTION::s_UL:
				UUSearch(p_tile, pivot->s_col, pivot->s_row - 1, pivot);
				ULSearch(p_tile, pivot->s_col - 1, pivot->s_row - 1, pivot);
				LLSearch(p_tile, pivot->s_col - 1, pivot->s_row, pivot);

				if (pivot->s_row + 1 < HEIGHT && 0 <= pivot->s_col - 1 && p_tile[pivot->s_row + 1][pivot->s_col].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row + 1][pivot->s_col - 1].status == RECTANGLE_STATUS::tail))
					DLSearch(p_tile, pivot->s_col - 1, pivot->s_row + 1, pivot);

				if (0 <= pivot->s_row - 1 && pivot->s_col + 1 < WIDTH && p_tile[pivot->s_row][pivot->s_col + 1].status == RECTANGLE_STATUS::wall && (p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::empty || p_tile[pivot->s_row - 1][pivot->s_col + 1].status == RECTANGLE_STATUS::tail))
					URSearch(p_tile, pivot->s_col + 1, pivot->s_row - 1, pivot);
				break;
		}
	}

	sort(m_Open_List.begin(), m_Open_List.end(), cmp);
	return false;
}

bool cmp(JUMPPOINTSEARCH::NODE* &u, JUMPPOINTSEARCH::NODE* &v)
{
	return u->s_F < v->s_F;
}


void JUMPPOINTSEARCH::UUSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int i = p_yPos; 0 <= i; i--)
	{
		if (CheckPath(p_tile, p_xPos, i) == false || cnt >= m_max_value)
			return;

		G_value += 1.0;
		if (p_tile[i][p_xPos].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_LL, p_xPos, i, G_value, p_ancestor);
			return;
		}
		else if ((0 <= i - 1 && 0 <= p_xPos - 1 && p_tile[i][p_xPos - 1].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][p_xPos - 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][p_xPos - 1].status == RECTANGLE_STATUS::tail))
			|| (0 <= i - 1 && p_xPos + 1 < WIDTH && p_tile[i][p_xPos + 1].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][p_xPos + 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][p_xPos + 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_UU, p_xPos, i, G_value, p_ancestor);
			return;
		}

		cnt++;
	}
}

void JUMPPOINTSEARCH::URSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int i = p_yPos, j = p_xPos; 0 <= i && j < WIDTH; i--, j++)
	{
		if (CheckPath(p_tile, j, i) == false || cnt >= m_max_value)
			return;

		G_value += 1.5f;
		if (p_tile[i][j].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_UR, j, i, G_value, p_ancestor);
			return;
		}
		else if ((0 <= i - 1 && 0 <= j - 1 && p_tile[i][j - 1].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][j - 1].status == RECTANGLE_STATUS::tail))
			|| (i + 1 < HEIGHT && j + 1 < WIDTH && p_tile[i + 1][j].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][j + 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_UR, j, i, G_value, p_ancestor);
			return;
		}

		int ycnt = 0;
		for (int ypos = i - 1; 0 <= ypos; ypos--)
		{
			if (CheckPath(p_tile, j, ypos) == false || ycnt >= m_max_value)
				break;

			if (p_tile[ypos][j].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_UR, j, i, G_value, p_ancestor);
				return;
			}
			else if ((0 <= ypos - 1 && 0 <= j - 1 && p_tile[ypos][j - 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos - 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[ypos - 1][j - 1].status == RECTANGLE_STATUS::tail))
				||(0 <= ypos - 1 && j + 1 < WIDTH && p_tile[ypos][j + 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos - 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[ypos - 1][j + 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_UR, j, i, G_value, p_ancestor);
				return;
			}

			ycnt++;
		}

		int xcnt = 0;
		for (int xpos = j + 1; xpos < WIDTH; xpos++)
		{
			if (CheckPath(p_tile, xpos, i) == false || xcnt >= m_max_value)
				break;

			if (p_tile[i][xpos].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_UR, j, i, G_value, p_ancestor);
				return;
			}
			else if ((0 <= i - 1 && xpos + 1 < WIDTH && p_tile[i - 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][xpos + 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][xpos + 1].status == RECTANGLE_STATUS::tail))
				|| (i + 1 < HEIGHT && xpos + 1 < WIDTH && p_tile[i + 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][xpos + 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][xpos + 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_UR, j, i, G_value, p_ancestor);
				return;
			}

			xcnt++;
		}

		cnt++;
	}
}

void JUMPPOINTSEARCH::RRSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int j = p_xPos; j < WIDTH; j++)
	{
		if (CheckPath(p_tile, j, p_yPos) == false || cnt >= m_max_value)
			return;

		G_value += 1.0;
		if (p_tile[p_yPos][j].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_LL, j, p_yPos, G_value, p_ancestor);
			return;
		}
		else if ((0 <= p_yPos - 1 && j + 1 < WIDTH  && p_tile[p_yPos - 1][j].status == RECTANGLE_STATUS::wall && (p_tile[p_yPos - 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[p_yPos - 1][j + 1].status == RECTANGLE_STATUS::tail))
			||(p_yPos + 1 < HEIGHT && j + 1 < WIDTH && p_tile[p_yPos + 1][j].status == RECTANGLE_STATUS::wall && (p_tile[p_yPos + 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[p_yPos + 1][j + 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_RR, j, p_yPos, G_value, p_ancestor);
			return;
		}

		cnt++;
	}
}

void JUMPPOINTSEARCH::DRSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int i = p_yPos, j = p_xPos; i < HEIGHT && j < WIDTH; i++, j++)
	{
		if (CheckPath(p_tile, j, i) == false || cnt >= m_max_value)
			return;

		G_value += 1.5f;
		if (p_tile[i][j].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_DR, j, i, G_value, p_ancestor);
			return;
		}
		else if ((0 <= i - 1 && j + 1 < WIDTH && p_tile[i - 1][j].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][j + 1].status == RECTANGLE_STATUS::tail))
			|| (i + 1 < HEIGHT && 0 <= j - 1 && p_tile[i][j - 1].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][j - 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_DR, j, i, G_value, p_ancestor);
			return;
		}

		int ycnt = 0;
		for (int ypos = i + 1; ypos < HEIGHT; ypos++)
		{
			if (CheckPath(p_tile, j, ypos) == false || ycnt >= m_max_value)
				break;

			if (p_tile[ypos][j].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_DR, j, i, G_value, p_ancestor);
				return;
			}
			else if ((ypos + 1 < HEIGHT && 0 <= j - 1 && p_tile[ypos][j - 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos + 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[ypos + 1][j - 1].status == RECTANGLE_STATUS::tail))
				|| (ypos + 1 < HEIGHT && j + 1 < WIDTH && p_tile[ypos][j + 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos + 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[ypos + 1][j + 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_DR, j, i, G_value, p_ancestor);
				return;
			}

			ycnt++;
		}

		int xcnt = 0;
		for (int xpos = j + 1; xpos < WIDTH; xpos++)
		{
			if (CheckPath(p_tile, xpos, i) == false || xcnt >= m_max_value)
				break;

			if (p_tile[i][xpos].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_DR, j, i, G_value, p_ancestor);
				return;
			}
			else if ((0 <= i - 1 && xpos + 1 < WIDTH && p_tile[i - 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][xpos + 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][xpos + 1].status == RECTANGLE_STATUS::tail))
				|| (i + 1 < HEIGHT && xpos + 1 < WIDTH && p_tile[i + 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][xpos + 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][xpos + 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_DR, j, i, G_value, p_ancestor);
				return;
			}

			xcnt++;
		}

		cnt++;
	}
}

void JUMPPOINTSEARCH::DDSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int i = p_yPos; i < HEIGHT; i++)
	{
		if (CheckPath(p_tile, p_xPos, i) == false || cnt >= m_max_value)
			return;

		G_value += 1.0;
		if (p_tile[i][p_xPos].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_LL, p_xPos, i, G_value, p_ancestor);
			return;
		}
		else if ((i + 1 < HEIGHT && 0 <= p_xPos - 1 && p_tile[i][p_xPos - 1].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][p_xPos - 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][p_xPos - 1].status == RECTANGLE_STATUS::tail))
			|| (i + 1 < HEIGHT && p_xPos + 1 < WIDTH && p_tile[i][p_xPos + 1].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][p_xPos + 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][p_xPos + 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_DD, p_xPos, i, G_value, p_ancestor);
			return;
		}

		cnt++;
	}
}

void JUMPPOINTSEARCH::DLSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int i = p_yPos, j = p_xPos; i < HEIGHT && 0 <= j; i++, j--)
	{
		if (CheckPath(p_tile, j, i) == false || cnt >= m_max_value)
			return;

		G_value += 1.5f;
		if (p_tile[i][j].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_DL, j, i, G_value, p_ancestor);
			return;
		}
		else if ((0 <= i - 1 && 0 <= j - 1 && p_tile[i - 1][j].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][j - 1].status == RECTANGLE_STATUS::tail))
			|| (i + 1 < HEIGHT && j + 1 < WIDTH && p_tile[i][j + 1].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][j + 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_DL, j, i, G_value, p_ancestor);
			return;
		}

		int ycnt = 0;
		for (int ypos = i + 1; ypos < HEIGHT; ypos++)
		{
			if (CheckPath(p_tile, j, ypos) == false || ycnt >= m_max_value)
				break;

			if (p_tile[ypos][j].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_DL, j, i, G_value, p_ancestor);
				return;
			}
			else if ((ypos + 1 < HEIGHT && 0 <= j - 1 && p_tile[ypos][j - 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos + 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[ypos + 1][j - 1].status == RECTANGLE_STATUS::tail))
				|| (ypos + 1 < HEIGHT && j + 1 < WIDTH && p_tile[ypos][j + 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos + 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[ypos + 1][j + 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_DL, j, i, G_value, p_ancestor);
				return;
			}

			ycnt++;
		}

		int xcnt = 0;
		for (int xpos = j - 1; 0 <= xpos; xpos--)
		{
			if (CheckPath(p_tile, xpos, i) == false || xcnt >= m_max_value)
				break;

			if (p_tile[i][xpos].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_DL, j, i, G_value, p_ancestor);
				return;
			}
			else if ((0 <= i - 1 && 0 <= xpos - 1 && p_tile[i - 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][xpos - 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][xpos - 1].status == RECTANGLE_STATUS::tail))
				|| (i + 1 < HEIGHT && 0 <= xpos - 1 && p_tile[i + 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][xpos - 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][xpos - 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_DL, j, i, G_value, p_ancestor);
				return;
			}

			xcnt++;
		}
		
		cnt++;
	}
}

void JUMPPOINTSEARCH::LLSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int j = p_xPos; 0 <= j; j--)
	{
		if (CheckPath(p_tile, j, p_yPos) == false || cnt >= m_max_value)
			return;

		G_value += 1.0;
		if (p_tile[p_yPos][j].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_LL, j, p_yPos, G_value, p_ancestor);
			return;
		}
		else if ((0 <= p_yPos - 1 && 0 <= j - 1 && p_tile[p_yPos - 1][j].status == RECTANGLE_STATUS::wall && (p_tile[p_yPos - 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[p_yPos - 1][j - 1].status == RECTANGLE_STATUS::tail))
			|| (p_yPos + 1 < HEIGHT && 0 <= j - 1 && p_tile[p_yPos + 1][j].status == RECTANGLE_STATUS::wall && (p_tile[p_yPos + 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[p_yPos + 1][j - 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_LL, j, p_yPos, G_value, p_ancestor);
			return;
		}

		cnt++;
	}
}

void JUMPPOINTSEARCH::ULSearch(vector< vector<NODE_INFO> > &p_tile, int p_xPos, int p_yPos, NODE *p_ancestor)
{
	int cnt = 0;
	float G_value = 0.0f;
	for (int i = p_yPos, j = p_xPos; 0 <= i && 0 <= j; i--, j--)
	{
		if (CheckPath(p_tile, j, i) == false || cnt >= m_max_value)
			return;
	
		G_value += 1.5f;
		if (p_tile[i][j].status == RECTANGLE_STATUS::tail)
		{
			MakeNode(p_tile, (char)DIRECTION::s_UL, j, i, G_value, p_ancestor);
			return;
		}
		else if ((0 <= i - 1 && j - 1 < WIDTH && p_tile[i][j + 1].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][j + 1].status == RECTANGLE_STATUS::tail))
			|| (i + 1 < HEIGHT && 0 <= j - 1 && p_tile[i + 1][j].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][j - 1].status == RECTANGLE_STATUS::tail)))
		{
			MakeNode(p_tile, (char)DIRECTION::s_UL, j, i, G_value, p_ancestor);
			return;
		}

		int ycnt = 0;
		for (int ypos = i - 1; 0 <= ypos; ypos--)
		{
			if (CheckPath(p_tile, j, ypos) == false || ycnt >= m_max_value)
				break;

			if (p_tile[ypos][j].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_UL, j, i, G_value, p_ancestor);
				return;
			}
			else if ((0 <= ypos - 1 && 0 <= j - 1 && p_tile[ypos][j - 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos - 1][j - 1].status == RECTANGLE_STATUS::empty || p_tile[ypos - 1][j - 1].status == RECTANGLE_STATUS::tail))
				|| (0 <= ypos - 1 && j + 1 < WIDTH && p_tile[ypos][j + 1].status == RECTANGLE_STATUS::wall && (p_tile[ypos - 1][j + 1].status == RECTANGLE_STATUS::empty || p_tile[ypos - 1][j + 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_UL, j, i, G_value, p_ancestor);
				return;
			}

			ycnt++;
		}

		int xcnt = 0;
		for (int xpos = j - 1; 0 <= xpos; xpos--)
		{
			if (CheckPath(p_tile, xpos, i) == false || xcnt >= m_max_value)
				break;

			if (p_tile[i][xpos].status == RECTANGLE_STATUS::tail)
			{
				MakeNode(p_tile, (char)DIRECTION::s_UL, j, i, G_value, p_ancestor);
				return;
			}
			else if ((0 <= i - 1 && 0 <= xpos - 1 && p_tile[i - 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i - 1][xpos - 1].status == RECTANGLE_STATUS::empty || p_tile[i - 1][xpos - 1].status == RECTANGLE_STATUS::tail))
				|| (i + 1 < HEIGHT && 0 <= xpos - 1 && p_tile[i + 1][xpos].status == RECTANGLE_STATUS::wall && (p_tile[i + 1][xpos - 1].status == RECTANGLE_STATUS::empty || p_tile[i + 1][xpos - 1].status == RECTANGLE_STATUS::tail)))
			{
				MakeNode(p_tile, (char)DIRECTION::s_UL, j, i, G_value, p_ancestor);
				return;
			}

			xcnt++;
		}

		cnt++;
	}
}