#ifndef LF_Stack_Info
#define LF_Stack_Info

#include "Log/Log.h"
#include "MemoryPool/MemoryPool.h"

#include <Windows.h>

namespace Olbbemi
{
	template<class T>
	class C_LFStack
	{
	private:
		struct ST_Node
		{
			T data;
			ST_Node* link;
		};

		struct ST_Top
		{
			volatile __int64 block_info[2];
		};

		volatile LONG m_use_count;
		__declspec(align(16)) ST_Top m_top;
		C_MemoryPool<ST_Node> *m_pool;

	public:
		C_LFStack()
		{
			m_pool = new C_MemoryPool<ST_Node>(0, false);
			m_top.block_info[0] = 0;	m_top.block_info[1] = 1;
		}

		void M_Push(T pa_data)
		{
			__declspec(align(16))__int64 lo_top[2];
			ST_Node* new_node = m_pool->M_Alloc();
			new_node->data = pa_data;	new_node->link = nullptr;

			do
			{
				lo_top[0] = m_top.block_info[0];	lo_top[1] = m_top.block_info[1];
				
				new_node->link = (ST_Node*)lo_top[0];
			} while (InterlockedCompareExchange128(m_top.block_info, lo_top[1] + 1, (LONG64)new_node, lo_top) == 0);

			InterlockedIncrement(&m_use_count);
		}

		T M_Pop()
		{
			if (InterlockedDecrement(&m_use_count) < 0)
			{
				TCHAR lo_action[] = _TEXT("LFStack"), lo_server[] = _TEXT("NONE");
				ST_Log lo_log({ "Pop: LFStack is NULL" });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}

			__declspec(align(16)) __int64 lo_top[2];
			T lo_return_value;
			ST_Node* lo_next = nullptr;

			do
			{
				lo_top[0] = m_top.block_info[0];	lo_top[1] = m_top.block_info[1];
				
				lo_next = ((ST_Node*)lo_top[0])->link;
				lo_return_value = ((ST_Node*)(lo_top[0]))->data;
			} while (InterlockedCompareExchange128(m_top.block_info, lo_top[1] + 1, (LONG64)lo_next, lo_top) == 0);

			m_pool->M_Free((ST_Node*)lo_top[0]);
			return lo_return_value;
		}

		LONG M_GetAllocCount() const
		{
			return m_pool->M_GetAllocCount();
		}
	};
}

#endif