#ifndef Astar_Info
#define Astar_Info

/*
	G : 현재 위치에서 출발지까지의 이동 비용
	H : 현재 위치에서 도착지까지의 예상 비용 ( manhattan, Euclid, Octile ... )
	F : G + H

	-> 외부에서 해당 클래스 생성하여 SetPostion 함수를 통해 시작 및 끝좌표를 전송
		Astar 함수를 통해 배열정보를 전송하여 경로 저장
*/

#include "Struct_Define.h"

#include <vector>
#include <deque>
#include <functional>
using namespace std;

class ASTAR
{
private:
	struct NODE
	{
		int s_row, s_col;
		float s_G, s_H, s_F;
		struct NODE *s_parent;
	};

	deque<NODE*> m_Open_List, m_Close_List;
	int m_startX, m_startY, m_endX, m_endY;

public:
	friend bool cmp(ASTAR::NODE* &u, ASTAR::NODE* &v);
	
	void SetPosition(int p_SxPos, int p_SyPos, int p_ExPos, int p_EyPos);
	bool Astar(vector< vector<INFO> > &p_shape, deque<TRACE*> &p_list);
};

#endif