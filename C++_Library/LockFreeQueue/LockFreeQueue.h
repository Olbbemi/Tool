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
			ST_Node* node_address;
			LONG64 unique_index;
		};

		volatile LONG m_queue_size;
		volatile __declspec(align(16)) ST_Ptr m_front, m_rear;
		C_MemoryPool<ST_Node> *m_pool;

	public:

		/**-------------------------------------------------------------------------------------------------------------------------------------------------------------------
		  * ������ ť���� ����� �޸� Ǯ�� �׻� �̸� �����Ǿ� �־�� �ϸ� �߰����� ��� ������ ���� ���ϵ��� ���� ( �߰����� ��尡 �߻��ϸ� �ش� ������ ������ ������ �Ǵ��ϱ� ���� )
		  * ������ ť�� Ư���� Front�� ����Ű�� ���� �׻� ���̳�带 �ǹ� ( �����ڿ��� �޸�Ǯ�κ��� ��带 �ϳ� �Ҵ� ���� )
		  *-------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
		C_LFQueue()
		{
			m_pool = new C_MemoryPool<ST_Node>(0, false);

			m_front.node_address = m_pool->M_Alloc();
			m_front.unique_index = 1;

			m_rear.node_address = m_front.node_address;
			m_rear.unique_index = 1;

			m_front.node_address->link = nullptr;
			m_queue_size = 0;
		}

		/**--------------------------------------------------------------------
		  * ���� ť�� ������ �ִ� ��带 ��� �޸� Ǯ�� ��ȯ �� ��� ī��Ʈ ����
		  * �޸� Ǯ ��ü ����
		  *--------------------------------------------------------------------*/
		~C_LFQueue()
		{
			ST_Node* lo_garbage;
			while (m_front.node_address != nullptr)
			{
				lo_garbage = m_front.node_address;
				m_front.node_address = m_front.node_address->link;
				m_pool->M_Free(lo_garbage);
				m_queue_size--;
			}

			m_rear.node_address = nullptr;
			delete m_pool;
		}

		/**---------------------------------------------------------------------------------------------------------------------------------------
		  * lo_rear �� lo_front ������ interlock128 �Լ����� ���ǹǷ� 16����Ʈ ������ �Ǿ� �־�� ��
		  * Rear �� ����Ű�� ��忡 ���ο� ��带 �����ϴ� �۾��� Rear �� �ű�� �۾��� ���� ( ���ο� ��尡 ����Ǳ⸸ �ϸ� ť ���� ��� ī��Ʈ�� ���� )
		  *---------------------------------------------------------------------------------------------------------------------------------------*/
		void M_Enqueue(T pa_data)
		{
			__declspec(align(16)) ST_Ptr lo_rear;

			ST_Node* lo_new_node = m_pool->M_Alloc();
			lo_new_node->data = pa_data;	lo_new_node->link = nullptr;

			while (1)
			{
				lo_rear.unique_index = m_rear.unique_index;
				lo_rear.node_address = m_rear.node_address;
				if (lo_rear.node_address->link == nullptr)
				{
					if (InterlockedCompareExchangePointer((PVOID*)&m_rear.node_address->link, lo_new_node, nullptr) == nullptr)
					{
						InterlockedIncrement(&m_queue_size);
						InterlockedCompareExchange128((LONG64*)&m_rear, lo_rear.unique_index + 1, (LONG64)lo_new_node, (LONG64*)&lo_rear);
						break;
					}
				}
				else
					InterlockedCompareExchange128((LONG64*)&m_rear, lo_rear.unique_index + 1, (LONG64)lo_rear.node_address->link, (LONG64*)&lo_rear);
			}
		}

		/**------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		  * lo_rear �� lo_front ������ interlock128 �Լ����� ���ǹǷ� 16����Ʈ ������ �Ǿ� �־�� ��
		  * ��忡 ���� ABA ������ ���� �� ���� ( Front->Next == nullptr ������ �ִµ� �̴� ũ�� ������ ���� �ʴ� ��Ȳ�̹Ƿ� continue �� ���� �ٽ� �õ��ϵ��� ����
		  * Front �� Rear �� �����鼭 Rear->Next �� nullptr �� �ƴ϶�� Enqueue ���� ������ ������ ������ Rear �̵������� ó������ ���� ��Ȳ�̹Ƿ� �ش� �Լ����� �ѹ��� ���� ( Front �� Rear �ڷ� ���� ������ ���� ���� )
		  *------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
		void M_Dequeue(T& pa_return_value)
		{
			__declspec(align(16)) ST_Ptr lo_front, lo_rear;

			while (1)
			{
				lo_front.unique_index = m_front.unique_index;
				lo_front.node_address = m_front.node_address;

				lo_rear.unique_index = m_rear.unique_index;
				lo_rear.node_address = m_rear.node_address;

				if (lo_front.node_address == lo_rear.node_address && lo_rear.node_address->link != nullptr)
				{
					InterlockedCompareExchange128((LONG64*)&m_rear, lo_rear.unique_index + 1, (LONG64)lo_rear.node_address->link, (LONG64*)&lo_rear);
					continue;
				}

				if (m_front.node_address == nullptr)
				{
					if (m_queue_size != 0)
						continue;
					else
					{
						TCHAR lo_action[] = _TEXT("LFQueue"), lo_server[] = _TEXT("Common");
						ST_Log lo_log({ "LFQueue is Null" });
						_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
					}
				}

				if (lo_front.node_address->link == nullptr || lo_front.node_address == lo_rear.node_address)
					continue;

				pa_return_value = lo_front.node_address->link->data;
				if (InterlockedCompareExchange128((LONG64*)&m_front, lo_front.unique_index + 1, (LONG64)lo_front.node_address->link, (LONG64*)&lo_front) == 1)
				{
					InterlockedDecrement(&m_queue_size);

					m_pool->M_Free(lo_front.node_address);
					break;
				}
			}
		}

		LONG M_GetAllocCount()
		{
			return m_pool->M_GetAllocCount();
		}

		LONG M_GetUseCount()
		{
			return m_queue_size;
		}
	};
}

#endif