#ifndef PDH_Info
#define PDH_Info

#include <pdh.h>
#include <vector>


#pragma comment(lib, "pdh.lib")

using namespace std;

namespace Olbbemi
{
	class C_PDH
	{
	private:
		TCHAR m_log_action[20], m_log_server[20];
		DWORD m_recv_item_count, m_recv_buffer_size, m_send_item_count, m_send_buffer_size;
		
		PDH_HQUERY	m_pdh_query;
		PDH_HCOUNTER m_pdh_counter_menoryhandle, m_pdh_counter_nonpagehandle, m_pdh_counter_send_net_handle, m_pdh_counter_recv_net_handle,
					 m_pdh_counter_chatting_memory;
		PDH_FMT_COUNTERVALUE_ITEM *m_send_item, *m_recv_item;

	public:
		C_PDH();
		~C_PDH();

		void M_CallCollectQuery();

		LONG M_GetChattingMemory();
		LONG M_GetNonpageData();
		LONG M_GetRemainMemory();
		void M_GetSendNetData(vector<LONG>& pa_store);
		void M_GetRecvNetData(vector<LONG>& pa_store);
	};
}

#endif