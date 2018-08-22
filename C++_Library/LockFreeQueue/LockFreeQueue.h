#ifndef LFQueue_Info
#define LFQueue_Info

#include "Log/Log.h"
#include "MemoryPool/MemoryPool.h"

#include <Windows.h>

namespace Olbbemi
{
	template<class T>
	class C_LFQueue
	{
	private:
		struct ST_Node
		{
			T data;
			ST_Node* link;
		};

		struct ST_Ptr
		{
			volatile __int64 block_info[2];
		};

		volatile __int64 m_use_count;
		__declspec(align(16)) ST_Ptr m_front, m_rear;
		C_MemoryPool<ST_Node> *m_pool;

	public:
		C_LFQueue()
		{
		   /**--------------------------------------------
			 * LFQueue Need Not More Alloc (new ST_Node X)
			 *--------------------------------------------*/
			m_pool = new C_MemoryPool<ST_Node>(4000, false);
			m_use_count = 0;

			m_front.block_info[0] = (LONG64)m_pool->M_Alloc();	m_front.block_info[1] = 1;
			m_rear.block_info[0] = m_front.block_info[0];	m_rear.block_info[1] = 1;

			((ST_Node*)m_front.block_info[0])->link = nullptr;
		}

		void M_Enqueue(T pa_data)
		{
			__declspec(align(16)) __int64 lo_rear[2], lo_front[2];

			ST_Node* lo_new_node = m_pool->M_Alloc();
			lo_new_node->data = pa_data;	lo_new_node->link = nullptr;
			
			while (1)
			{
				lo_rear[1] = m_rear.block_info[1];
				lo_rear[0] = m_rear.block_info[0];	
				if (((ST_Node*)lo_rear[0])->link == nullptr)
				{
					if (InterlockedCompareExchangePointer((PVOID*)&((ST_Node*)m_rear.block_info[0])->link, lo_new_node, nullptr) == nullptr)
					{
						InterlockedIncrement64(&m_use_count);
						InterlockedCompareExchange128(m_rear.block_info, lo_rear[1] + 1, (LONG64)lo_new_node, lo_rear)
						break;
					}
				}
				else
					InterlockedCompareExchange128(m_rear.block_info, lo_rear[1] + 1, (LONG64)(((ST_Node*)lo_rear[0])->link), lo_rear)
			}
		}

		bool M_Dequeue(T& pa_return_value)
		{
			ST_Node *lo_next;

			__declspec(align(16)) __int64 lo_front[2], lo_rear[2];

			while (1)
			{
				lo_front[1] = m_front.block_info[1];
				lo_front[0] = m_front.block_info[0];

				lo_rear[1] = m_rear.block_info[1];
				lo_rear[0] = m_rear.block_info[0];		

				if (lo_front[0] == lo_rear[0] && ((ST_Node*)lo_rear[0])->link != nullptr)
				{
					if (InterlockedCompareExchange128(m_rear.block_info, lo_rear[1] + 1, (LONG64)(((ST_Node*)lo_rear[0])->link), lo_rear) == 1)
						InterlockedIncrement64(&move_tail);
					
					continue;
				}

				if (m_front.block_info[0] == 0)
				{
					if(m_use_count != 0)
						continue;
				}

				lo_next = ((ST_Node*)lo_front[0])->link;
				if (lo_next == nullptr || lo_front[0] == lo_rear[0])
					continue;

				pa_return_value = lo_next->data;
				if (InterlockedCompareExchange128(m_front.block_info, lo_front[1] + 1, (LONG64)(((ST_Node*)lo_front[0])->link), lo_front) == 1)
				{
					InterlockedDecrement64(&m_use_count);

					bool check = m_pool->M_Free((ST_Node*)lo_front[0]);
					if (check == false)
						return false;
					
					break;
				}
			}
			
			return true;
		}

		LONG M_GetAllocCount()
		{
			return m_pool->M_GetAllocCount();
		}
	};
}

#endif