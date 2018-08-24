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
		  * 락프리 큐에서 템플릿으로 전달된 자료형을 가지는 데이터를 저장할 노드 구조체
		  * 단방향 링크드 리스트형식이므로 다음 노드를 저장할 포인터를 가짐
		  *------------------------------------------------------------------------*/
		struct ST_Node
		{
			T data;
			ST_Node* link;
		};

		/**-----------------------------------------------------------------
		  * 락프리 큐에서 노드 정보를 저장할 Front 및 Rear 변수가 사용할 구조체
		  * 락프리 큐의 ABA 문제를 해결하기 위해 128bit 사용
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
		  * 락프리 큐에서 사용할 메모리 풀은 항상 미리 생성되어 있어야 하며 추가적인 노드 생성은 하지 못하도록 설정 ( 추가적인 노드가 발생하면 해당 세션이 문제가 있음을 판단하기 위함 )
		  * 락프리 큐의 특성상 Front가 가리키는 노드는 항상 더미노드를 의미 ( 생성자에서 메모리풀로부터 노드를 하나 할당 받음 )
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
	  	  * 현재 큐가 가지고 있는 노드를 모두 메모리 풀에 반환 및 사용 카운트 감소
		  * 메모리 풀 객체 삭제
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
		  * lo_rear 및 lo_front 변수는 interlock128 함수에서 사용되므로 16바이트 정렬이 되어 있어야 함
		  * Rear 가 가리키는 노드에 새로운 노드를 연결하는 작업과 Rear 를 옮기는 작업이 별도 ( 새로운 노드가 연결되기만 하면 큐 내부 사용 카운트를 증가 )
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
		  * lo_rear 및 lo_front 변수는 interlock128 함수에서 사용되므로 16바이트 정렬이 되어 있어야 함
		  * 노드에 대한 ABA 문제가 있을 수 있음 ( Front->Next == nullptr 현상이 있는데 이는 크게 문제가 되지 않는 상황이므로 continue 를 통해 다시 시도하도록 유도
		  * Front 와 Rear 이 같으면서 Rear->Next 가 nullptr 이 아니라면 Enqueue 에서 연결은 성공을 했으나 Rear 이동연산은 처리하지 못한 상황이므로 해당 함수에서 한번더 수행 ( Front 가 Rear 뒤로 가는 현상을 막기 위함 )
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