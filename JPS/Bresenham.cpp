#include "Precompile.h"
#include "Bresenham.h"

bool BRESENHAM::Bresenham(vector< vector<NODE_INFO> > &p_tile, int p_SxPos, int p_SyPos, int p_ExPos, int p_EyPos)
{
	int error, delta_x, delta_y, xpos, ypos;
	pair<int, int> start_spot, end_spot;

	if (p_SxPos < p_ExPos)
	{
		start_spot.first = p_SxPos;	start_spot.second = p_SyPos;
		end_spot.first = p_ExPos;	end_spot.second = p_EyPos;
	}
	else
	{
		start_spot.first = p_ExPos;	start_spot.second = p_EyPos;
		end_spot.first = p_SxPos;	end_spot.second = p_SyPos;
	}

	delta_x = abs(end_spot.first - start_spot.first);
	delta_y = abs(end_spot.second - start_spot.second);

	if (delta_x < delta_y)
	{
		xpos = start_spot.first;
		error = delta_y / 2;
		if (start_spot.second < end_spot.second)
		{
			for (int i = start_spot.second; i <= start_spot.second + delta_y; i++)
			{
				if (error >= delta_y)
				{
					xpos++;
					error -= delta_y;
				}

				if (p_tile[i][xpos].status == RECTANGLE_STATUS::wall)
					return false;

				error += delta_x;
			}
		}
		else
		{
			for (int i = start_spot.second; i >= start_spot.second - delta_y; i--)
			{
				if (error >= delta_y)
				{
					xpos++;
					error -= delta_y;
				}

				if (p_tile[i][xpos].status == RECTANGLE_STATUS::wall)
					return false;

				error += delta_x;
			}
		}
	}
	else
	{
		ypos = start_spot.second;
		error = delta_x / 2;

		if (start_spot.second < end_spot.second)
		{	
			for (int j = start_spot.first; j <= start_spot.first + delta_x; j++)
			{
				if (error >= delta_x)
				{
					ypos++;
					error -= delta_x;
				}

				if (p_tile[ypos][j].status == RECTANGLE_STATUS::wall)
					return false;

				error += delta_y;
			}
		}
		else
		{
			for (int j = start_spot.first; j <= start_spot.first + delta_x; j++)
			{
				if (error >= delta_x)
				{
					ypos--;
					error -= delta_x;
				}

				if (p_tile[ypos][j].status == RECTANGLE_STATUS::wall)
					return false;

				error += delta_y;
			}
		}
	}

	return true;
}