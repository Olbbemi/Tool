#ifndef Jump_Point_Search_NODE_INFO
#define Jump_Point_Search_NODE_INFO

#include "Struct_Define.h"

#include <Windows.h>
#include <vector>
#include <list>
#include <deque>
#include <functional>
using namespace std;

class BRESENHAM;

class JUMPPOINTSEARCH
{
private:
	enum class DIRECTION : char
	{
		s_UU = 0,
		s_UR,
		s_RR,
		s_DR,
		s_DD,
		s_DL,
		s_LL,
		s_UL,
	};

	struct NODE
	{
		char s_direction;
		int s_row, s_col;
		float s_G, s_H, s_F;
		
		struct NODE *s_parent;
	};

	list<NODE*> m_Close_List;
	deque<NODE*> m_Open_List;
	int m_endX, m_endY, m_max_value;

	BRESENHAM *m_bresenham;

public:
	friend bool cmp(JUMPPOINTSEARCH::NODE* &u, JUMPPOINTSEARCH::NODE* &v);

	JUMPPOINTSEARCH(int p_search_max);

	void SetPosition(vector< vector<NODE_INFO> > &p_shape, int p_SxPos, int p_SyPos, int p_ExPos, int p_EyPos);
	bool JPS(vector< vector<NODE_INFO> > &shape, list<TRACE_INFO*> &p_trace, int p_color);
	void MakeNode(vector< vector<NODE_INFO> > &p_shape, char p_direction, int p_xPos, int p_yPos, float p_G, NODE *ancestor);
	list<TRACE_INFO*> MakeTrace(vector< vector<NODE_INFO> > &p_tile, NODE *p_pivot);
	bool inline CheckPath(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos);

	void UUSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void URSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void RRSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void DRSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void DDSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void DLSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void LLSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
	void ULSearch(vector< vector<NODE_INFO> > &p_shape, int p_xPos, int p_yPos, NODE *p_ancestor);
};

#endif