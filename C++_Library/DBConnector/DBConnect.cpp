#include "Precompile.h"
#include "DBConnect.h"

#include "Log/Log.h"

#include <tchar.h>
#include <strsafe.h>

#define MAX_TRY 5

using namespace Olbbemi;

/**-----------------------------------------------------------------------------
  * NetServer 의 Worker Thread 만큼 DB 에 연결할 수 있도록 DBConnector 객체를 생성
  *-----------------------------------------------------------------------------*/
C_DBConnectTLS::C_DBConnectTLS(int pa_make_worker_count, TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port)
{
	m_index = -1;
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

/**-----------------------------------------------------------------------------------------------------
  * NetServer 의 Worker Thread 에서 DB 접근을 필요로 할 때마다 해당 함수를 호출하여 DBConnector 객체를 얻음
  *-----------------------------------------------------------------------------------------------------*/
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

/**---------------------------------------------------------------
  * 각 DBConnector 객체마다 연결할 DB 정보를 갱신하고 해당 DB 에 연결
  *---------------------------------------------------------------*/
void C_DBConnector::M_Initialize(TCHAR* pa_DB_ip, TCHAR* pa_DB_user, TCHAR* pa_DB_password, TCHAR* pa_DB_name, int pa_DB_port)
{
	mysql_init(&m_mysql);

	WideCharToMultiByte(CP_UTF8, 0, pa_DB_ip, sizeof(m_DB_ip), m_DB_ip, sizeof(m_DB_ip), NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pa_DB_user, sizeof(m_DB_user), m_DB_user, sizeof(m_DB_user), NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pa_DB_password, sizeof(m_DB_password), m_DB_password, sizeof(m_DB_password), NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pa_DB_name, sizeof(m_DB_name), m_DB_name, sizeof(m_DB_name), NULL, NULL);
	m_DB_port = pa_DB_port;

	M_Connect();
}

C_DBConnector::C_DBConnector()
{
	StringCchCopy(m_log_action, _countof(m_log_action), _TEXT("DBConnector"));
	StringCchCopy(m_log_server, _countof(m_log_server), _TEXT("Common"));
}

C_DBConnector::~C_DBConnector()
{
	mysql_close(m_connection);
}

/**---------------------------------------------------------
  * DB 에 연결 시도 후 일정횟수 이상 실패하면 로그 및 서버 종료
  *---------------------------------------------------------*/
void C_DBConnector::M_Connect()
{
	for (int i = 0; i < MAX_TRY; i++)
	{
		m_connection = mysql_real_connect(&m_mysql, m_DB_ip, m_DB_user, m_DB_password, m_DB_name, m_DB_port, (char *)NULL, 0);
		if (m_connection != NULL)
		{
			mysql_set_character_set(m_connection, "utf8");
			return;
		}
		else
		{
			int lo_error = mysql_errno(&m_mysql);
			
			m_last_error = lo_error;
			strcpy_s(m_last_error_msg, sizeof(m_last_error_msg), mysql_error(&m_mysql));

			if (lo_error != CR_SOCKET_CREATE_ERROR &&  lo_error != CR_CONNECTION_ERROR && 
				lo_error != CR_CONN_HOST_ERROR &&  lo_error != CR_SERVER_GONE_ERROR && 
				lo_error != CR_SERVER_HANDSHAKE_ERR &&  lo_error != CR_SERVER_LOST &&  lo_error != CR_INVALID_CONN_HANDLE)
			{
				ST_Log lo_log({ "DB Connect Error [" + to_string(lo_error) + ", " + m_last_error_msg + "]" });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
			}			
		}
	}
	
	ST_Log lo_log({ "DB Connect Try Max Over" });
	_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
}

/**---------------------------------------------------------------------
  * DB 에 질의를 필요로하는 함수에서 해당 함수를 호출
  * pa_is_write : false -> Select 쿼리문으로 반환값을 요구 o
  * pa_is_write : true  -> Insert 및 Update 등 쿼리문으로 반환값을 요구 x
  *---------------------------------------------------------------------*/
void C_DBConnector::M_Query(bool pa_is_write, vector<MYSQL_ROW>& pa_sql_row, string pa_string_format, int pa_count, ...)
{
	char lo_str[1024];
	va_list lo_vlist;
	MYSQL_ROW lo_sql_row;
	
	va_start(lo_vlist, pa_count);
	StringCchVPrintfA(lo_str, _countof(lo_str), pa_string_format.c_str(), lo_vlist);

	while (1)
	{
		int lo_check = mysql_query(m_connection, lo_str);
		if (lo_check == FALSE)
			break;
		else
		{
			int lo_error = mysql_errno(&m_mysql);
			if (lo_error == CR_SOCKET_CREATE_ERROR || lo_error == CR_CONNECTION_ERROR ||
				lo_error == CR_CONN_HOST_ERROR || lo_error == CR_SERVER_GONE_ERROR ||
				lo_error == CR_SERVER_HANDSHAKE_ERR || lo_error == CR_SERVER_LOST || lo_error == CR_INVALID_CONN_HANDLE)
			{
				M_Connect();
			}
			else
			{
				string lo_via_str = lo_str;
				ST_Log lo_log({ "Query : " + lo_via_str });
				_LOG(__LINE__, LOG_LEVEL_QUERY, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
			}
		}
	}
	
	if (pa_is_write == false)
	{
		m_sql_result = mysql_store_result(m_connection);
		while ((lo_sql_row = mysql_fetch_row(m_sql_result)) != NULL)
			pa_sql_row.push_back(lo_sql_row);
	}
}

/**---------------------------------------------------------------
  * M_Query 함수를 호출한 위치에서 해당 함수가 종료되기전 반드시 호출
  *---------------------------------------------------------------*/
void C_DBConnector::M_FreeResult()
{
	mysql_free_result(m_sql_result);
}