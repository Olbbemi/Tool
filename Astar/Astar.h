#ifndef Astar_Info
#define Astar_Info

/*
	G : ���� ��ġ���� ����������� �̵� ���
	H : ���� ��ġ���� ������������ ���� ��� ( manhattan, Euclid, Octile ... )
	F : G + H

	-> �ܺο��� �ش� Ŭ���� �����Ͽ� SetPostion �Լ��� ���� ���� �� ����ǥ�� ����
		Astar �Լ��� ���� �迭������ �����Ͽ� ��� ����
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