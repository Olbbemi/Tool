#ifndef LF_Stack_Info
#define LF_Stack_Info

#include "Log/Log.h"
#include "MemoryPool/MemoryPool.h"

namespace Olbbemi
{
	/*
	*/

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

		__int64 m_unique_index;
		__declspec(align(16)) ST_Top m_top;
		C_MemoryPool<ST_Node> m_pool;

	public:
		C_LFStack()
		{
			m_unique_index = 1;
			m_top.block_info[0] = 0;	m_top.block_info[1] = 0;
		}

		void M_Push(T pa_data)
		{
			__int64 lo_top;
			ST_Node* new_node = m_pool.Alloc();
			new_node->data = pa_data;	new_node->link = nullptr;

			do
			{
				lo_top = m_top.block_info[0];
				new_node->link = (ST_Node*)lo_top;
			} while (InterlockedCompareExchange64((LONG64*)&m_top.block_info[0], (LONG64)new_node, lo_top) != lo_top);
		}

		T M_Pop()
		{
			__declspec(align(16)) __int64 lo_top[2];
			LONG64 lo_value = InterlockedIncrement64(&m_unique_index);
			ST_Node* lo_next = nullptr;

			do
			{
				lo_top[0] = 0;	lo_top[1] = 0;
				InterlockedCompareExchange128(m_top.block_info, 0, 0, lo_top);

				if (lo_top[0] == 0)
				{
					TCHAR lo_action[] = _TEXT("LFStack"), lo_server[] = _TEXT("NONE");
					ST_Log lo_log({ "Pop: LFStack is NULL" });
					_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
				}

				lo_next = ((ST_Node*)lo_top[0])->link;
			} while (InterlockedCompareExchange128(m_top.block_info, lo_value, (LONG64)lo_next, lo_top) == 0);

			m_pool.Free((ST_Node*)lo_top[0]);
			return ((ST_Node*)(lo_top[0]))->data;
		}
	};
}

#endif