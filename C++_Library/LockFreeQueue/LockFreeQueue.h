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

		/**------------------------------------------------------------------------
		  * ������ ť���� ���ø����� ���޵� �ڷ����� ������ �����͸� ������ ��� ����ü
		  * �ܹ��� ��ũ�� ����Ʈ�����̹Ƿ� ���� ��带 ������ �����͸� ����
		  *------------------------------------------------------------------------*/
		struct ST_Node
		{
			T data;
			ST_Node* link;
		};

		/**-----------------------------------------------------------------
		  * ������ ť���� ��� ������ ������ Front �� Rear ������ ����� ����ü
		  * ������ ť�� ABA ������ �ذ��ϱ� ���� 128bit ���
		  * [0]: Node Address, [1]: Unique Index
		  *-----------------------------------------------------------------*/
		struct ST_Ptr
		{
			volatile LONG64 block_info[2];
		};

		volatile LONG64 m_use_count;
		__declspec(align(16)) ST_Ptr m_front, m_rear;
		C_MemoryPool<ST_Node> *m_pool;

	public:

		/**-------------------------------------------------------------------------------------------------------------------------------------------------------------------
		  * ������ ť���� ����� �޸� Ǯ�� �׻� �̸� �����Ǿ� �־�� �ϸ� �߰����� ��� ������ ���� ���ϵ��� ���� ( �߰����� ��尡 �߻��ϸ� �ش� ������ ������ ������ �Ǵ��ϱ� ���� )
		  * ������ ť�� Ư���� Front�� ����Ű�� ���� �׻� ���̳�带 �ǹ� ( �����ڿ��� �޸�Ǯ�κ��� ��带 �ϳ� �Ҵ� ���� )
		  *-------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
		C_LFQueue()
		{
			m_pool = new C_MemoryPool<ST_Node>(1000, false);
			
			m_front.block_info[0] = (LONG64)m_pool->M_Alloc();	
			m_front.block_info[1] = 1;
			
			m_rear.block_info[0] = m_front.block_info[0];	
			m_rear.block_info[1] = 1;

			((ST_Node*)m_front.block_info[0])->link = nullptr;
			m_use_count = 0;
		}

		/**-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	  	  * ���� ť�� ������ �ִ� ��带 ��� �޸� Ǯ�� ��ȯ �� ��� ī��Ʈ ����
		  * �޸� Ǯ ��ü ����
		  *-------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
		~C_LFQueue()
		{
			ST_Node* lo_garbage;
			while (m_front.block_info[0] != 0)
			{
				lo_garbage = (ST_Node*)m_front.block_info[0];
				m_front.block_info[0] = (LONG64)(((ST_Node*)m_front.block_info[0])->link);
				m_pool->M_Free(lo_garbage);
				m_use_count--;
			}

			m_rear[0] = 0;
			delete m_pool;
		}

		/**---------------------------------------------------------------------------------------------------------------------------------------
		  * lo_rear �� lo_front ������ interlock128 �Լ����� ���ǹǷ� 16����Ʈ ������ �Ǿ� �־�� ��
		  * Rear �� ����Ű�� ��忡 ���ο� ��带 �����ϴ� �۾��� Rear �� �ű�� �۾��� ���� ( ���ο� ��尡 ����Ǳ⸸ �ϸ� ť ���� ��� ī��Ʈ�� ���� )
		  *---------------------------------------------------------------------------------------------------------------------------------------*/
		void M_Enqueue(T pa_data)
		{
			__declspec(align(16)) LONG64 lo_rear[2], lo_front[2];

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

		/**------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		  * lo_rear �� lo_front ������ interlock128 �Լ����� ���ǹǷ� 16����Ʈ ������ �Ǿ� �־�� ��
		  * ��忡 ���� ABA ������ ���� �� ���� ( Front->Next == nullptr ������ �ִµ� �̴� ũ�� ������ ���� �ʴ� ��Ȳ�̹Ƿ� continue �� ���� �ٽ� �õ��ϵ��� ����
		  * Front �� Rear �� �����鼭 Rear->Next �� nullptr �� �ƴ϶�� Enqueue ���� ������ ������ ������ Rear �̵������� ó������ ���� ��Ȳ�̹Ƿ� �ش� �Լ����� �ѹ��� ���� ( Front �� Rear �ڷ� ���� ������ ���� ���� )
		  *------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
		void M_Dequeue(T& pa_return_value)
		{
			ST_Node *lo_next;
			__declspec(align(16)) LONG64 lo_front[2], lo_rear[2];

			while (1)
			{
				lo_front[1] = m_front.block_info[1];
				lo_front[0] = m_front.block_info[0];

				lo_rear[1] = m_rear.block_info[1];
				lo_rear[0] = m_rear.block_info[0];		

				if (lo_front[0] == lo_rear[0] && ((ST_Node*)lo_rear[0])->link != nullptr)
				{
					InterlockedCompareExchange128(m_rear.block_info, lo_rear[1] + 1, (LONG64)(((ST_Node*)lo_rear[0])->link), lo_rear)
					continue;
				}

				if (m_front.block_info[0] == 0)
				{
					if(m_use_count != 0)
						continue;
					else
					{
						TCHAR lo_action[] = _TEXT("LFQueue"), lo_server[] = _TEXT("NONE");
						ST_Log lo_log({ "LFQueue is Null" });
						_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
					}
				}

				lo_next = ((ST_Node*)lo_front[0])->link;
				if (lo_next == nullptr || lo_front[0] == lo_rear[0])
					continue;

				pa_return_value = lo_next->data;
				if (InterlockedCompareExchange128(m_front.block_info, lo_front[1] + 1, (LONG64)(((ST_Node*)lo_front[0])->link), lo_front) == 1)
				{
					InterlockedDecrement64(&m_use_count);

					m_pool->M_Free((ST_Node*)lo_front[0]);	
					break;
				}
			}
		}

		LONG M_GetAllocCount()
		{
			return m_pool->M_GetAllocCount();
		}
	};
}

#endif