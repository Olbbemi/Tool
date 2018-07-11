#ifndef Bresenham_Info
#define Bresenham_Info

#include "Struct_Define.h"

#include <vector>
using namespace std;

class BRESENHAM
{
public:
	bool Bresenham(vector< vector<NODE_INFO> > &p_shape, int p_SxPos, int p_SyPos, int p_ExPos, int p_EyPos);
};

#endif