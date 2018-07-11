#include "Precompile.h"
#include "Astar.h"

#include <algorithm>

bool cmp(ASTAR::NODE* &u, ASTAR::NODE* &v);

void ASTAR::SetPosition(int p_SxPos, int p_SyPos, int p_ExPos, int p_EyPos)
{
	m_Open_List.clear();
	m_Close_List.clear();

	m_startX = p_SxPos;	m_endX = p_ExPos;
	m_startY = p_SyPos;	m_endY = p_EyPos;

	NODE *new_node = new NODE;
	new_node->s_parent = nullptr;
	new_node->s_row = m_startY;
	new_node->s_col = m_startX;
	new_node->s_G = 0.0;
	new_node->s_H = ((float)abs(p_ExPos - p_SxPos) + (float)abs(p_EyPos - p_SyPos));
	new_node->s_F = new_node->s_G + new_node->s_H;

	m_Open_List.push_back(new_node);
}

bool ASTAR::Astar(vector< vector<INFO> > &p_shape, deque<TRACE*> &p_list)
{
	 pair<int, int> direction[8] = { {-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1} };

	 if (m_Open_List.size() == 0) // open_list 존재하지 않으면 더이상 경로 없음을 의미	
		 return true;
	 
	 NODE *pivot = m_Open_List.front();	m_Open_List.pop_front();

	 m_Close_List.push_back(pivot);
	 if ((pivot->s_col != m_startX || pivot->s_row != m_startY))
		 p_shape[pivot->s_row][pivot->s_col].status = RECTANGLE_STATUS::close_list;
		 
	 for (int i = 0; i < 8; i++)
	 {
		 bool flag = false;
		 int Xtemp = direction[i].second + pivot->s_col, Ytemp = direction[i].first + pivot->s_row;

		 if ((0 <= Ytemp && Ytemp < HEIGHT) && (0 <= Xtemp  && Xtemp < WIDTH) && p_shape[Ytemp][Xtemp].status != RECTANGLE_STATUS::wall)
		 {
			 // 목적지에 도달함을 의미
			 if (Xtemp == m_endX && Ytemp == m_endY)
			 {
				 NODE *new_node = new NODE;
				 new_node->s_col = Xtemp;
				 new_node->s_row = Ytemp;
				 new_node->s_parent = pivot;

				while (new_node->s_parent != nullptr)
				{
					TRACE *trace = new TRACE;

					trace->curXpos = (p_shape[new_node->s_row][new_node->s_col].startX + p_shape[new_node->s_row][new_node->s_col].endX) >> 1;
					trace->curYpos = (p_shape[new_node->s_row][new_node->s_col].startY + p_shape[new_node->s_row][new_node->s_col].endY) >> 1;
					trace->preXpos = (p_shape[new_node->s_parent->s_row][new_node->s_parent->s_col].startX + p_shape[new_node->s_parent->s_row][new_node->s_parent->s_col].endX) >> 1;
					trace->preYpos = (p_shape[new_node->s_parent->s_row][new_node->s_parent->s_col].startY + p_shape[new_node->s_parent->s_row][new_node->s_parent->s_col].endY) >> 1;

					p_list.push_front(trace);
					new_node = new_node->s_parent;
				}

				 return true;
			 }

			for (auto list = m_Close_List.begin(); list != m_Close_List.end(); list++)
			{
				if ((*list)->s_col == Xtemp && (*list)->s_row == Ytemp)
				{
					if ((i & 1) == 1 && (*list)->s_G > pivot->s_G + 1.5f)
					{
						(*list)->s_G = pivot->s_G + 1.5f;
						(*list)->s_F = (*list)->s_G + (*list)->s_H;
						(*list)->s_parent = pivot;
					}
					else if ((i & 1) == 0 && (*list)->s_G > pivot->s_G + 1.0f)
					{
						(*list)->s_G = pivot->s_G + 1.0f;
						(*list)->s_F = (*list)->s_G + (*list)->s_H;
						(*list)->s_parent = pivot;
					}

					flag = true;
					break;
				}
			}
			if (flag == true) // close_list 에 노드가 존재하면 스킵
				continue;


			for (auto list = m_Open_List.begin(); list != m_Open_List.end(); list++)
			{
				if ((*list)->s_col == Xtemp && (*list)->s_row == Ytemp)
				{
					if ((i & 1) == 1 && (*list)->s_G > pivot->s_G + 1.5f)
					{
						(*list)->s_G = pivot->s_G + 1.5f;
						
						(*list)->s_F = (*list)->s_G + (*list)->s_H;
						
						(*list)->s_parent = pivot;
					}
					else if( (i & 1) == 0 && (*list)->s_G > pivot->s_G + 1.0f)
					{
						(*list)->s_G = pivot->s_G + 1.0f;
						
						(*list)->s_F = (*list)->s_G + (*list)->s_H;
						
						(*list)->s_parent = pivot;
					}

					flag = true;
					break;
				}
			}
			if (flag == true) // open_list 에 노드가 존재하면 스킵
				continue;

			 if (flag == false)
			 {
				 NODE *new_node = new NODE;
				 new_node->s_col = Xtemp;
				 new_node->s_row = Ytemp;
				 new_node->s_parent = pivot;

				 if ((i & 1) == 1)
					 new_node->s_G = pivot->s_G + 1.5f;
				 else
					 new_node->s_G = pivot->s_G + 1.0f;

				 new_node->s_H = ((float)abs(Xtemp - m_endX) + (float)abs(Ytemp - m_endY));
				 new_node->s_F = new_node->s_G + new_node->s_H;
				 m_Open_List.push_back(new_node);

				 p_shape[Ytemp][Xtemp].status = RECTANGLE_STATUS::open_list;
			 }
		 }
	 }

	 sort(m_Open_List.begin(), m_Open_List.end(), cmp);
	 return false;
}

bool cmp(ASTAR::NODE* &u, ASTAR::NODE* &v)
{
	return u->s_F < v->s_F;
}