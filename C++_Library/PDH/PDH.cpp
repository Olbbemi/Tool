#include "Precompile.h"
#include "PDH.h"

#include "Log/Log.h"

#include <strsafe.h>
#include <pdhmsg.h>

using namespace Olbbemi;

/**----------------------------------
  * PDH 를 이용하여 원하는 쿼리를 추출하기 위해 리소스를 생성
  * PdhOpenQuery: 
  * PdhAddCounter:
  * PdhCollectQueryData:
  * PdhGetFormattedCounterArray: 
  *----------------------------------*/
C_PDH::C_PDH()
{
	StringCchCopy(m_log_action, _countof(m_log_action), _TEXT("PDH"));
	StringCchCopy(m_log_server, _countof(m_log_server), _TEXT("Common"));

	m_recv_item_count = m_recv_buffer_size = 0; 
	m_send_item_count = m_send_buffer_size = 0;
	m_recv_item = m_send_item = nullptr;

	if (PdhOpenQuery(NULL, NULL, &m_pdh_query) != ERROR_SUCCESS)
	{
		ST_Log lo_log({"PDH Open Error Code: " + to_string(GetLastError())});
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}
	
	if (PdhAddCounter(m_pdh_query, _TEXT("\\Memory\\Pool Nonpaged Bytes"), NULL, &m_pdh_counter_nonpagehandle) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "NonPage Pool AddCounter Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhAddCounter(m_pdh_query, _TEXT("\\Memory\\Available MBytes"), NULL, &m_pdh_counter_menoryhandle) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Available Byte AddCounter Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhAddCounter(m_pdh_query, _TEXT("\\Network Interface(*)\\Bytes Received/sec"), NULL, &m_pdh_counter_recv_net_handle) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Byte Recv AddCounter Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhAddCounter(m_pdh_query, _TEXT("\\Network Interface(*)\\Bytes Sent/sec"), NULL, &m_pdh_counter_send_net_handle) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Byte Send AddCounter Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhAddCounter(m_pdh_query, _TEXT("\\Process(ChattingServer)\\Private Bytes"), NULL, &m_pdh_counter_chatting_memory) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Byte Send AddCounter Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhCollectQueryData(m_pdh_query) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "CollectQuery Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhGetFormattedCounterArray(m_pdh_counter_send_net_handle, PDH_FMT_LONG, &m_send_buffer_size, &m_send_item_count, m_send_item) == PDH_MORE_DATA)
	{
		m_send_item = (PDH_FMT_COUNTERVALUE_ITEM*)malloc(m_send_buffer_size);
		if (m_send_item == nullptr)
		{
			ST_Log lo_log({ "Malloc Fail" });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
		}
	}
	else
	{
		ST_Log lo_log({ "PdhGetFormattedCounterArray Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	if (PdhGetFormattedCounterArray(m_pdh_counter_recv_net_handle, PDH_FMT_LONG, &m_recv_buffer_size, &m_recv_item_count, m_recv_item) == PDH_MORE_DATA)
	{
		m_recv_item = (PDH_FMT_COUNTERVALUE_ITEM*)malloc(m_recv_buffer_size);
		if (m_recv_item == nullptr)
		{
			ST_Log lo_log({ "Malloc Fail" });
			_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
		}
	}
	else
	{
		ST_Log lo_log({ "PdhGetFormattedCounterArray Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}
}

C_PDH::~C_PDH()
{
	PdhCloseQuery(m_pdh_query);
}

/**----------------------------------------------------------
  * 등록한 쿼리를 수행해 해당 쿼리에 맞는 데이터를 추출하는 함수
  * 데이터를 얻어야 하는 곳에서 해당 함수 호출
  *----------------------------------------------------------*/
void C_PDH::M_CallCollectQuery()
{
	if (PdhCollectQueryData(m_pdh_query) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "CollectQuery Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}
}

/**----------------------------------------
  * 채팅서버의 메모리 사용량을 얻기 위한 함수
  *----------------------------------------*/
LONG C_PDH::M_GetChattingMemory()
{
	PDH_FMT_COUNTERVALUE lo_counter_value;

	if (PdhGetFormattedCounterValue(m_pdh_counter_chatting_memory, PDH_FMT_LONG, NULL, &lo_counter_value) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Nonpage_PdhGetFormattedCounterValue Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	return lo_counter_value.longValue;
}

/**-----------------------------------------------
  * 현재 프로세서의 논페이지 사용량을 얻기 위한 함수
  *-----------------------------------------------*/
LONG C_PDH::M_GetNonpageData()
{
	PDH_FMT_COUNTERVALUE lo_counter_value;

	if (PdhGetFormattedCounterValue(m_pdh_counter_nonpagehandle, PDH_FMT_LONG, NULL, &lo_counter_value) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Nonpage_PdhGetFormattedCounterValue Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	return lo_counter_value.longValue;
}

/**-----------------------------------------------
  * 현재 프로세서의 사용가능 메모리를 얻기 위한 함수
  *-----------------------------------------------*/
LONG C_PDH::M_GetRemainMemory()
{
	PDH_FMT_COUNTERVALUE lo_counter_value;

	if (PdhGetFormattedCounterValue(m_pdh_counter_menoryhandle, PDH_FMT_LONG, NULL, &lo_counter_value) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Memory_PdhGetFormattedCounterValue Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}

	return lo_counter_value.longValue;
}

/**-----------------------------------------------
  * 현재 프로세서의 송신 네트워크량을 얻기 위한 함수
  *-----------------------------------------------*/
void C_PDH::M_GetSendNetData(vector<LONG>& pa_store)
{
	if (PdhGetFormattedCounterArray(m_pdh_counter_send_net_handle, PDH_FMT_LONG, &m_send_buffer_size, &m_send_item_count, m_send_item) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Send_PdhGetFormattedCounterValue Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}
	else
	{
		for (int i = 0; i < m_send_item_count; i++)
			pa_store.push_back(m_send_item[i].FmtValue.longValue);
	}	
}

/**-----------------------------------------------
  * 현재 프로세서의 수신 네트워크량을 얻기 위한 함수
  *-----------------------------------------------*/
void C_PDH::M_GetRecvNetData(vector<LONG>& pa_store)
{
	if (PdhGetFormattedCounterArray(m_pdh_counter_recv_net_handle, PDH_FMT_LONG, &m_recv_buffer_size, &m_recv_item_count, m_recv_item) != ERROR_SUCCESS)
	{
		ST_Log lo_log({ "Recv_PdhGetFormattedCounterValue Error Code: " + to_string(GetLastError()) });
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, m_log_action, m_log_server, lo_log.count, lo_log.log_str);
	}
	else
	{
		for (int i = 0; i < m_recv_item_count; i++)
			pa_store.push_back(m_recv_item[i].FmtValue.longValue);
	}
}