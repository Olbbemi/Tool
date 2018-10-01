#ifndef DB_Connect_Info
#define DB_Connect_Info

#include <Windows.h>
#include "mysqlConnector/include/errmsg.h"
#include "mysqlConnector/include/mysql.h"

#include <vector>
#include <string>

#pragma comment(lib, "lib/vs14/mysqlclient.lib")

using namespace std;

namespace Olbbemi
{
	class C_DBConnector
	{
	private:
		TCHAR m_log_action[20], m_log_server[20];

		char m_DB_ip[16], m_DB_user[64], m_DB_password[64], m_DB_name[64], m_last_error_msg[128];
		int m_DB_port, m_last_error;

		MYSQL m_mysql, *m_connection;
		MYSQL_RES* m_sql_result;

		void M_Initialize(TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port);
		void M_Connect();

		friend class C_DBConnectTLS;

	public:
		C_DBConnector();
		~C_DBConnector();

		void M_Query(bool pa_is_write, vector<MYSQL_ROW>& pa_sql_row, string pa_string_format, int pa_count, ...);
		void M_FreeResult();
	};
	 
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
}

#endif