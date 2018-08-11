#include "Precompile.h"
#include "Sector.h"

#include <algorithm>

bool cmp(const pair<int, int> &u, const pair<int, int> &v);

SECTOR::SECTOR()
{
	m_width = TOTAL_GAME_SIZE / SECTOR_WIDTH_SIZE + 1;
	m_height = TOTAL_GAME_SIZE / SECTOR_HEIGHT_SIZE + 1;
	
	m_session_array.resize(m_height);
	for (int i = 0; i < m_height; i++)
		m_session_array[i].resize(m_width);
}

SECTOR::~SECTOR()
{
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			// �迭���� �����ϴ� ������� ����
			for (auto session_list = m_session_array[i][j].begin(); session_list != m_session_array[i][j].end();)
				m_session_array[i][j].erase(session_list++);
		}

		m_session_array[i].clear();
	}

	m_session_array.clear();
}

void SECTOR::SetUnitSectorPosition(Session_Info *p_session)
{
	// �ش� ������ ���� ���Ϳ� ���� ���Ͱ��� ����
	p_session->s_pre_Sector_width = p_session->s_cur_Sector_width;	p_session->s_pre_Sector_height = p_session->s_cur_Sector_height;

	// �ش� ������ ���� ���Ͱ� ����
	p_session->s_cur_Sector_width = p_session->s_width / SECTOR_WIDTH_SIZE;	p_session->s_cur_Sector_height = p_session->s_height / SECTOR_HEIGHT_SIZE;

	/*
		�ش� ������ ���� ���Ͱ��� -1�̸� �ش� ������ ó�� �����Ȱ��� �ǹ��ϹǷ� ���� ���Ͱ��� �ش� ���������� ����
	
		�ش� ������ ���� ���Ͱ��� ���� ���Ͱ��� �ٸ��� ���� ���Ͱ��� �ش� �������� ����, ���� ���Ͱ��� �ش� �������� ����
	*/
	if (p_session->s_pre_Sector_width == -1 && p_session->s_pre_Sector_height == -1)
		m_session_array[p_session->s_cur_Sector_height][p_session->s_cur_Sector_width].insert(make_pair(p_session->s_socket, p_session));
	else if (p_session->s_pre_Sector_width != p_session->s_cur_Sector_width || p_session->s_pre_Sector_height != p_session->s_cur_Sector_height)
	{
		m_session_array[p_session->s_pre_Sector_height][p_session->s_pre_Sector_width].erase(p_session->s_socket);
		m_session_array[p_session->s_cur_Sector_height][p_session->s_cur_Sector_width].insert(make_pair(p_session->s_socket, p_session));
	}
}

void SECTOR::GetUnitTotalSector(Session_Info *p_session, list<Session_Info*> &p_user_list)
{
	pair<int, int> direction[8] = { {-1, 0},{-1, 1},{0, 1},{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1} };
	
	// �ش� ������ ��ġ�� ���Ϳ� �����ϴ� ������� ����
	int width_value = p_session->s_cur_Sector_width, height_value = p_session->s_cur_Sector_height;
	for (auto user_list = m_session_array[height_value][width_value].begin(); user_list != m_session_array[height_value][width_value].end(); ++user_list)
		p_user_list.push_back((*user_list).second);

	// �ش� ������ ��ġ�� ���͸� �������� 8���⿡ �����ϴ� ������� ����
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

void SECTOR::GetUnitVariationSector(Session_Info *p_session, list<Session_Info*> &p_old_sector_user_list, list<Session_Info*> &p_new_sector_user_list)
{
	vector< pair<int, int> > m_pre_sector_list, m_cur_sector_list, m_pre_result_sector_list, m_cur_result_sector_list;

	m_pre_sector_list.reserve(5000), m_cur_sector_list.reserve(5000), m_pre_result_sector_list.reserve(5000), m_cur_result_sector_list.reserve(5000);
	pair<int, int> direction[8] = { { -1, 0 },{ -1, 1 },{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 1, -1 },{ 0, -1 },{ -1, -1 } };

	// �ش������� ���� ���� ����� ����
	m_pre_sector_list.push_back(make_pair(p_session->s_pre_Sector_height, p_session->s_pre_Sector_width));

	// �ش������� ���� ���͸� �������� 8���� ����� ����
	int old_width_value = p_session->s_pre_Sector_width, old_height_value = p_session->s_pre_Sector_height;
	for (int i = 0; i < 8; i++)
	{
		int Htemp = old_height_value + direction[i].first, Wtemp = old_width_value + direction[i].second;
		if (0 <= Wtemp && Wtemp < m_width && 0 <= Htemp && Htemp < m_height)
			m_pre_sector_list.push_back(make_pair(Htemp, Wtemp));
	}

	// �ش������� ���� ���� ����� ����
	m_cur_sector_list.push_back(make_pair(p_session->s_cur_Sector_height, p_session->s_cur_Sector_width));

	// �ش������� ���� ���͸� �������� 8���� ����� ����
	int new_width_value = p_session->s_cur_Sector_width, new_height_value = p_session->s_cur_Sector_height;
	for (int i = 0; i < 8; i++)
	{
		int Htemp = new_height_value + direction[i].first, Wtemp = new_width_value + direction[i].second;
		if (0 <= Wtemp && Wtemp < m_width && 0 <= Htemp && Htemp < m_height)
			m_cur_sector_list.push_back(make_pair(Htemp, Wtemp));
	}

	sort(m_pre_sector_list.begin(), m_pre_sector_list.end(), cmp);
	sort(m_cur_sector_list.begin(), m_cur_sector_list.end(), cmp);
	
	// �������� �̿��Ͽ� ���ŵ� ���ͺκ��� �˻� 
	auto cSet = set_difference(m_pre_sector_list.begin(), m_pre_sector_list.end(), m_cur_sector_list.begin(), m_cur_sector_list.end(), m_pre_result_sector_list.begin());
	m_pre_result_sector_list.erase(cSet, m_pre_result_sector_list.end());

	for (auto sector_list = m_pre_result_sector_list.begin(); sector_list != m_pre_result_sector_list.end(); ++sector_list)
	{
		for (auto user_list = m_session_array[(*sector_list).first][(*sector_list).second].begin(); user_list != m_session_array[(*sector_list).first][(*sector_list).second].end(); ++user_list)
			p_old_sector_user_list.push_back((*user_list).second);
	}
		
	// �������� �̿��Ͽ� ���� �߰��� ���ͺκ��� �˻�
	cSet = set_difference(m_cur_sector_list.begin(), m_cur_sector_list.end(), m_pre_sector_list.begin(), m_pre_sector_list.end(), m_cur_result_sector_list.begin());
	m_cur_result_sector_list.erase(cSet, m_cur_result_sector_list.end());

	for (auto sector_list = m_cur_result_sector_list.begin(); sector_list != m_cur_result_sector_list.end(); ++sector_list)
	{
		for (auto user_list = m_session_array[(*sector_list).first][(*sector_list).second].begin(); user_list != m_session_array[(*sector_list).first][(*sector_list).second].end(); ++user_list)
			p_new_sector_user_list.push_back((*user_list).second);
	}
}

void SECTOR::DeleteUnitSector(Session_Info *p_session)
{
	m_session_array[p_session->s_cur_Sector_height][p_session->s_cur_Sector_width].erase(p_session->s_socket);
}

void SECTOR::PrintSector()
{
	// ����� �κ� ������Ͽ� �ڵ� ¥��
}

bool cmp(const pair<int, int> &u, const pair<int, int> &v)
{
	if (u.first == v.first)
		return u.second < v.second;
	else
		return u.first < v.first;
}