#include "Precompile.h"
#include "DBConnect.h"

#include "Log/Log.h"

#include <tchar.h>
#include <strsafe.h>

#define MAX_TRY 5


using namespace Olbbemi;

C_DBConnectTLS::C_DBConnectTLS(int pa_make_worker_count, TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port)
{
	m_index = 0;
	m_tls_index = TlsAlloc();
	m_chunk = new C_DBConnector[pa_make_worker_count];

	for (int i = 0; i < pa_make_worker_count; i++)
		m_chunk[i].M_Initialize(pa_DB_ip, pa_DB_user, pa_DB_password, pa_DB_name, pa_DB_port);
}

C_DBConnectTLS::~C_DBConnectTLS()
{
	TlsFree(m_tls_index);
	delete m_chunk;
}

C_DBConnector* C_DBConnectTLS::M_GetPtr()
{
	void* lo_ptr = TlsGetValue(m_tls_index);
	if (lo_ptr == nullptr)
	{
		lo_ptr = &m_chunk[InterlockedIncrement(&m_index)];
		TlsSetValue(m_tls_index, lo_ptr);
	}

	return (C_DBConnector*)lo_ptr;
}

void C_DBConnector::M_Initialize(TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port)
{
	mysql_init(&m_mysql);

	WideCharToMultiByte(CP_UTF8, 0, pa_DB_ip, _countof(pa_DB_ip), m_DB_ip, sizeof(m_DB_ip), NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pa_DB_user, _countof(pa_DB_user), m_DB_user, sizeof(m_DB_user), NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pa_DB_password, _countof(pa_DB_password), m_DB_password, sizeof(m_DB_password), NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pa_DB_name, _countof(pa_DB_name), m_DB_name, sizeof(m_DB_name), NULL, NULL);
	m_DB_port = pa_DB_port;

	for (int i = 0; i < MAX_TRY; i++)
	{
		bool lo_check = M_Connect();
		if (lo_check == true)
			break;
		else
		{
			int lo_errno = M_GetLastErrorNo();
			if (lo_errno == CR_SOCKET_CREATE_ERROR || CR_CONNECTION_ERROR || CR_CONN_HOST_ERROR || CR_SERVER_GONE_ERROR
						 || CR_SERVER_HANDSHAKE_ERR || CR_SERVER_LOST || CR_INVALID_CONN_HANDLE)
			{
				continue;
			}
			else
			{
				string lo_error_str = M_GetLastErrorMsg();
				ST_Log lo_log({ "DB Connect ErrMsg: " + lo_error_str });

				_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
			}
		}
	}
}

C_DBConnector::~C_DBConnector()
{

}


bool C_DBConnector::M_Connect()
{
	m_connection = mysql_real_connect(&m_mysql, m_DB_ip, m_DB_user, m_DB_password, m_DB_name, m_DB_port, (char *)NULL, 0);
	if (m_connection == NULL)
	{
		m_last_error = mysql_errno(&m_mysql);
		strcpy_s(m_last_error_msg, sizeof(m_last_error_msg), mysql_error(&m_mysql));
		return false;
	}

	mysql_set_character_set(m_connection, "utf8");
	return true;
}

bool C_DBConnector::M_Disconnect()
{
	mysql_close(m_connection);
}

bool C_DBConnector::M_Query(string pa_string_format, initializer_list<LONG64> pa_init_list, vector<MYSQL_ROW>& pa_sql_row, bool pa_is_write)
{
	MYSQL_ROW lo_sql_row;
	

	//pa_init_list.begin();
	// log




	mysql_query(m_connection, pa_string_format.c_str());

	if (pa_is_write == false)
	{
		m_sql_result = mysql_store_result(m_connection);
		while ((lo_sql_row = mysql_fetch_row(m_sql_result)) != NULL)
			pa_sql_row.push_back(lo_sql_row);
	}
}

void C_DBConnector::M_FreeResult()
{
	mysql_free_result(m_sql_result);
}

int	C_DBConnector::M_GetLastErrorNo()
{ 
	return m_last_error;
};

char* C_DBConnector::M_GetLastErrorMsg()
{
	return m_last_error_msg;
}