#ifndef Sector_Info
#define Sector_Info

#include "Protocol/StructProtocol.h"

#include <vector>
#include <unordered_map>
using namespace std;

#define TOTAL_GAME_SIZE 6400
#define SECTOR_WIDTH_SIZE 720
#define SECTOR_HEIGHT_SIZE 520

namespace Olbbemi 
{
	class C_Sector
	{
	private:
		
		int m_width, m_height;
		vector< vector< unordered_map<LONG64, ST_Player*> > > m_session_array;

	public:
		C_Sector();
		~C_Sector();

		void SetUnitSectorPosition(ST_Player *p_session);
		void GetUnitTotalSector(ST_Player *p_session, vector<ST_Player*> &p_user_list);
		void GetUnitVariationSector(ST_Player *p_session, vector<ST_Player*> &p_old_sector_user_list, vector<ST_Player*> &p_new_sector_user_list);
		void DeleteUnitSector(ST_Player *p_session);
		void PrintSector();
	};
}

#endif