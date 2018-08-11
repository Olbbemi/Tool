#ifndef Sector_Info
#define Sector_Info

/*
 * 컨텐츠에 따라 추가해야할 구조체가 다르며 Sector 클래스 내부에 사용하는 STL 및 Define 값도 컨텐츠마다 다르게 적용해야 함
 */

#include "Struct_Define.h"

#include <list>
#include <vector>
#include <map>
using namespace std;

#define TOTAL_GAME_SIZE 6400
#define SECTOR_WIDTH_SIZE 720
#define SECTOR_HEIGHT_SIZE 520

namespace Olbbemi 
{
	class SECTOR
	{
	private:
		int m_width, m_height;
		vector< vector< map<SOCKET, Session_Info*> > > m_session_array;

	public:
		SECTOR();
		~SECTOR();

		void SetUnitSectorPosition(Session_Info *p_session);
		void GetUnitTotalSector(Session_Info *p_session, list<Session_Info*> &p_user_list);
		void GetUnitVariationSector(Session_Info *p_session, list<Session_Info*> &p_old_sector_user_list, list<Session_Info*> &p_new_sector_user_list);
		void DeleteUnitSector(Session_Info *p_session);
		void PrintSector();
	};
}

#endif