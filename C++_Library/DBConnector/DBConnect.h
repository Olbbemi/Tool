#ifndef DB_Connect_Info
#define DB_Connect_Info

#include <Windows.h>
#include "mysqlConnector/include/errmsg.h"
#include "mysqlConnector/include/mysql.h"

#include <vector>
#include <string>
#include <initializer_list>

using namespace std;

namespace Olbbemi
{
	class C_DBConnector;
	 
	class C_DBConnectTLS
	{
	private:
		DWORD m_tls_index;
		volatile LONG m_index;
		
		C_DBConnector* m_chunk;

	public:
		C_DBConnectTLS(int pa_make_worker_count, TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port);
		~C_DBConnectTLS();

		C_DBConnector* M_GetPtr();
	};


	class C_DBConnector
	{
	private:
		char m_DB_ip[16], m_DB_user[64], m_DB_password[64], m_DB_name[64], m_last_error_msg[128];
		int m_DB_port, m_last_error;

		MYSQL m_mysql, *m_connection;
		MYSQL_RES* m_sql_result;

		void M_Initialize(TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port);

		bool M_Connect();
		bool M_Disconnect();

		int	M_GetLastErrorNo();
		char* M_GetLastErrorMsg();

		friend class C_DBConnectTLS;

	public:
		~C_DBConnector();

		bool M_Query(string pa_string_format, initializer_list<LONG64> pa_init_list, vector<MYSQL_ROW>& pa_sql_row, bool pa_is_write = false);
		void M_FreeResult();
	};
}

#endif