#include "Precompile.h"
#include "Sector.h"

#include <algorithm>

using namespace Olbbemi;

/**------------------------------------------------------------------------------------
  * 전체 맵의 크기와 섹터의 크기를 얻어와 전체 섹터 개수를 결정한 뒤 해당 개수만큼 배열 생성
  *------------------------------------------------------------------------------------*/
C_Sector::C_Sector()
{
	m_width = TOTAL_GAME_SIZE / SECTOR_WIDTH_SIZE + 1;
	m_height = TOTAL_GAME_SIZE / SECTOR_HEIGHT_SIZE + 1;
	
	m_session_array.resize(m_height);
	for (int i = 0; i < m_height; i++)
		m_session_array[i].resize(m_width);
}

/**---------------------------------------------------
  * 각 섹터내에 존재하는 모든 세션 삭제 및 섹터 배열 정리
  *---------------------------------------------------*/
C_Sector::~C_Sector()
{
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			for (auto session_list = m_session_array[i][j].begin(); session_list != m_session_array[i][j].end();)
				m_session_array[i][j].erase(session_list++);
		}

		m_session_array[i].clear();
	}

	m_session_array.clear();
}

/**------------------------------------------------------------------------------------------------------------
  * 해당 세션이 위치할 섹터를 계산하여 해당 섹터에 삽입하는 함수
  * 이전 위치와 현재 위치가 다르다면 이전 위치의 섹터에서는 해당 세션을 삭제, 현재 위치의 섹터에서는 해당 세션을 삽입
  * 이전 위치가 -1이라면 해당 세션이 처음 생성된 것을 의미함
  * 연산한 좌표가 주어진 배열인덱스 범위를 벗어나면 Disconnect 함수 호출하도록 유도
  *------------------------------------------------------------------------------------------------------------*/
bool C_Sector::SetUnitSectorPosition(ST_PLAYER *p_session)
{
	p_session->pre_width_index = p_session->cur_width_index;	
	p_session->pre_height_index = p_session->cur_height_index;

	p_session->cur_width_index = p_session->xpos / SECTOR_WIDTH_SIZE;	p_session->cur_height_index = p_session->ypos / SECTOR_HEIGHT_SIZE;

	if (p_session->cur_width_index < 0 || p_session->cur_height_index < 0 || m_width <= p_session->cur_width_index || m_height <= p_session->cur_height_index)
		return false;

	if (p_session->pre_width_index == -1 && p_session->pre_height_index == -1)
		m_session_array[p_session->cur_height_index][p_session->cur_width_index].insert(make_pair(p_session->session_id, p_session));
	else if (p_session->pre_width_index != p_session->cur_width_index || p_session->pre_height_index != p_session->cur_height_index)
	{
		m_session_array[p_session->pre_height_index][p_session->pre_width_index].erase(p_session->session_id);
		m_session_array[p_session->cur_height_index][p_session->cur_width_index].insert(make_pair(p_session->session_id, p_session));
	}
}

/**----------------------------------------------------------------------------------------
  * 해당 세션이 위치한 섹터와 인접한 8방향의 섹터에 존재하는 모든 세션을 얻기 위해 호출하는 함수
  *----------------------------------------------------------------------------------------*/
void C_Sector::GetUnitTotalSector(ST_PLAYER *p_session, vector<ST_PLAYER*> &p_user_list)
{
	pair<int, int> direction[8] = { {-1, 0},{-1, 1},{0, 1},{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1} };
	
	int width_value = p_session->cur_width_index, height_value = p_session->cur_height_index;
	for (auto user_list = m_session_array[height_value][width_value].begin(); user_list != m_session_array[height_value][width_value].end(); ++user_list)
		p_user_list.push_back((*user_list).second);

	for (int i = 0; i < 8; i++)
	{
		int Htemp = height_value + direction[i].first, Wtemp = width_value + direction[i].second;
		if (0 <= Wtemp && Wtemp < m_width && 0 <= Htemp && Htemp < m_height)
		{
			for (auto user_list = m_session_array[Htemp][Wtemp].begin(); user_list != m_session_array[Htemp][Wtemp].end(); ++user_list)
				p_user_list.push_back((*user_list).second);
		}
	}
}

/**--------------------------------------------------------
  * 해당 세션이 종료될 때 섹터에서 제거하기 위해 호출하는 함수
  *--------------------------------------------------------*/
void C_Sector::DeleteUnitSector(ST_PLAYER *p_session)
{
	m_session_array[p_session->cur_height_index][p_session->cur_width_index].erase(p_session->session_id);
}