#include "Precompile.h"
//#include "Sector.h"
//
//#include <algorithm>
//
//using namespace Olbbemi;
//
//bool cmp(const pair<int, int> &u, const pair<int, int> &v);
//
//C_Sector::C_Sector()
//{
//	m_width = TOTAL_GAME_SIZE / SECTOR_WIDTH_SIZE + 1;
//	m_height = TOTAL_GAME_SIZE / SECTOR_HEIGHT_SIZE + 1;
//	
//	m_session_array.resize(m_height);
//	for (int i = 0; i < m_height; i++)
//		m_session_array[i].resize(m_width);
//}
//
//C_Sector::~C_Sector()
//{
//	for (int i = 0; i < m_height; i++)
//	{
//		for (int j = 0; j < m_width; j++)
//		{
//			// 배열내에 존재하는 유저목록 삭제
//			for (auto session_list = m_session_array[i][j].begin(); session_list != m_session_array[i][j].end();)
//				m_session_array[i][j].erase(session_list++);
//		}
//
//		m_session_array[i].clear();
//	}
//
//	m_session_array.clear();
//}
//
//void C_Sector::SetUnitSectorPosition(ST_User_Info *p_session)
//{
//	// 해당 유저의 이전 섹터에 현재 섹터값을 대입
//	p_session->s_pre_Sector_width = p_session->s_cur_Sector_width;	p_session->s_pre_Sector_height = p_session->s_cur_Sector_height;
//
//	// 해당 유저의 현재 섹터값 갱신
//	p_session->s_cur_Sector_width = p_session->s_width / SECTOR_WIDTH_SIZE;	p_session->s_cur_Sector_height = p_session->s_height / SECTOR_HEIGHT_SIZE;
//
//	/*
//		해당 유저의 이전 섹터값이 -1이면 해당 유저가 처음 생성된것을 의미하므로 현재 섹터값에 해당 유저정보를 대입
//	
//		해당 유저의 이전 섹터값과 현재 섹터값이 다르면 이전 섹터값에 해당 유저정보 삭제, 현재 섹터값에 해당 유저정보 대입
//	*/
//	if (p_session->s_pre_Sector_width == -1 && p_session->s_pre_Sector_height == -1)
//		m_session_array[p_session->s_cur_Sector_height][p_session->s_cur_Sector_width].insert(make_pair(p_session->s_socket, p_session));
//	else if (p_session->s_pre_Sector_width != p_session->s_cur_Sector_width || p_session->s_pre_Sector_height != p_session->s_cur_Sector_height)
//	{
//		m_session_array[p_session->s_pre_Sector_height][p_session->s_pre_Sector_width].erase(p_session->s_socket);
//		m_session_array[p_session->s_cur_Sector_height][p_session->s_cur_Sector_width].insert(make_pair(p_session->s_socket, p_session));
//	}
//}
//
//void C_Sector::GetUnitTotalSector(ST_User_Info *p_session, vector<ST_User_Info*> &p_user_list)
//{
//	pair<int, int> direction[8] = { {-1, 0},{-1, 1},{0, 1},{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1} };
//	
//	// 해당 유저가 위치한 섹터에 존재하는 유저목록 저장
//	int width_value = p_session->s_cur_Sector_width, height_value = p_session->s_cur_Sector_height;
//	for (auto user_list = m_session_array[height_value][width_value].begin(); user_list != m_session_array[height_value][width_value].end(); ++user_list)
//		p_user_list.push_back((*user_list).second);
//
//	// 해당 유저가 위치한 섹터를 기준으로 8방향에 존재하는 유저목록 저장
//	for (int i = 0; i < 8; i++)
//	{
//		int Htemp = height_value + direction[i].first, Wtemp = width_value + direction[i].second;
//		if (0 <= Wtemp && Wtemp < m_width && 0 <= Htemp && Htemp < m_height)
//		{
//			for (auto user_list = m_session_array[Htemp][Wtemp].begin(); user_list != m_session_array[Htemp][Wtemp].end(); ++user_list)
//				p_user_list.push_back((*user_list).second);
//		}
//	}
//}
//
//void C_Sector::GetUnitVariationSector(ST_User_Info *p_session, vector<ST_User_Info*> &p_old_sector_user_list, vector<ST_User_Info*> &p_new_sector_user_list)
//{
//	vector< pair<int, int> > m_pre_sector_list, m_cur_sector_list, m_pre_result_sector_list, m_cur_result_sector_list;
//
//	m_pre_sector_list.reserve(5000), m_cur_sector_list.reserve(5000), m_pre_result_sector_list.reserve(5000), m_cur_result_sector_list.reserve(5000);
//	pair<int, int> direction[8] = { { -1, 0 },{ -1, 1 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 1, -1 },{ 0, -1 },{ -1, -1 } };
//
//	// 해당유저의 이전 섹터 목록을 저장
//	m_pre_sector_list.push_back(make_pair(p_session->s_pre_Sector_height, p_session->s_pre_Sector_width));
//
//	// 해당유저의 이전 섹터를 기준으로 8방향 목록을 저장
//	int old_width_value = p_session->s_pre_Sector_width, old_height_value = p_session->s_pre_Sector_height;
//	for (int i = 0; i < 8; i++)
//	{
//		int Htemp = old_height_value + direction[i].first, Wtemp = old_width_value + direction[i].second;
//		if (0 <= Wtemp && Wtemp < m_width && 0 <= Htemp && Htemp < m_height)
//			m_pre_sector_list.push_back(make_pair(Htemp, Wtemp));
//	}
//
//	// 해당유저의 현재 섹터 목록을 저장
//	m_cur_sector_list.push_back(make_pair(p_session->s_cur_Sector_height, p_session->s_cur_Sector_width));
//
//	// 해당유저의 현재 섹터를 기준으로 8방향 목록을 저장
//	int new_width_value = p_session->s_cur_Sector_width, new_height_value = p_session->s_cur_Sector_height;
//	for (int i = 0; i < 8; i++)
//	{
//		int Htemp = new_height_value + direction[i].first, Wtemp = new_width_value + direction[i].second;
//		if (0 <= Wtemp && Wtemp < m_width && 0 <= Htemp && Htemp < m_height)
//			m_cur_sector_list.push_back(make_pair(Htemp, Wtemp));
//	}
//
//	sort(m_pre_sector_list.begin(), m_pre_sector_list.end(), cmp);
//	sort(m_cur_sector_list.begin(), m_cur_sector_list.end(), cmp);
//	
//	// 차집합을 이용하여 제거될 섹터부분을 검색 
//	auto cSet = set_difference(m_pre_sector_list.begin(), m_pre_sector_list.end(), m_cur_sector_list.begin(), m_cur_sector_list.end(), m_pre_result_sector_list.begin());
//	m_pre_result_sector_list.erase(cSet, m_pre_result_sector_list.end());
//
//	for (auto sector_list = m_pre_result_sector_list.begin(); sector_list != m_pre_result_sector_list.end(); ++sector_list)
//	{
//		for (auto user_list = m_session_array[(*sector_list).first][(*sector_list).second].begin(); user_list != m_session_array[(*sector_list).first][(*sector_list).second].end(); ++user_list)
//			p_old_sector_user_list.push_back((*user_list).second);
//	}
//		
//	// 차집합을 이용하여 새로 추가된 섹터부분을 검색
//	cSet = set_difference(m_cur_sector_list.begin(), m_cur_sector_list.end(), m_pre_sector_list.begin(), m_pre_sector_list.end(), m_cur_result_sector_list.begin());
//	m_cur_result_sector_list.erase(cSet, m_cur_result_sector_list.end());
//
//	for (auto sector_list = m_cur_result_sector_list.begin(); sector_list != m_cur_result_sector_list.end(); ++sector_list)
//	{
//		for (auto user_list = m_session_array[(*sector_list).first][(*sector_list).second].begin(); user_list != m_session_array[(*sector_list).first][(*sector_list).second].end(); ++user_list)
//			p_new_sector_user_list.push_back((*user_list).second);
//	}
//}
//
//void C_Sector::DeleteUnitSector(ST_User_Info *p_session)
//{
//	m_session_array[p_session->s_cur_Sector_height][p_session->s_cur_Sector_width].erase(p_session->s_socket);
//}
//
//void C_Sector::PrintSector()
//{
//	// 출력할 부분 재검증하여 코드 짜기
//}
//
//bool cmp(const pair<int, int> &u, const pair<int, int> &v)
//{
//	if (u.first == v.first)
//		return u.second < v.second;
//	else
//		return u.first < v.first;
//}