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

		/**--------------------------------------------------------------------------
		  * 락프리 스택에서 템플릿으로 전달된 자료형을 가지는 데이터를 저장할 노드 구조체
		  * 단방향 링크드 리스트형식이므로 다음 노드를 저장할 포인터를 가짐
		  *--------------------------------------------------------------------------*/
		struct ST_Node
		{
			T data;
			ST_Node* link;
		};

		/**-------------------------------------------------------------------
		  * 락프리 스택에서 노드 정보를 저장할 Front 및 Rear 변수가 사용할 구조체
		  * 락프리 스택의 ABA 문제를 해결하기 위해 128bit 사용
		  * [0]: Node Address, [1]: Unique Index
		  *-------------------------------------------------------------------*/
		struct ST_Top
		{
			ST_Node* node_address;
			LONG64 unique_index;
		};

		volatile LONG m_stack_size;
		volatile __declspec(align(16)) ST_Top m_top;
		C_MemoryPool<ST_Node> *m_pool;

	public:

		C_LFStack()
		{
			m_pool = new C_MemoryPool<ST_Node>(0, false);

			m_top.node_address = 0;
			m_top.unique_index = 1;
		}

		/**---------------------------------------------------------------------
		  * 현재 스택이 가지고 있는 모든 노드를 메모리 풀에 반환 및 사용 카운트 감소
		  * 메모리 풀 객체 삭제
		  *---------------------------------------------------------------------*/
		~C_LFStack()
		{
			ST_Node* lo_garbage;
			while (m_top.node_address != 0)
			{
				m_stack_size--;
				lo_garbage = m_top.node_address;
				m_top.node_address = m_top.node_address->link;
				m_pool->M_Free(lo_garbage);
			}

			delete m_pool;
		}

		/**----------------------------------------------------------------------------
		  * 락프리 스택에서 Push 함수는 ABA 상황을 유발하지 않으므로 interlock64 함수 사용
		  *----------------------------------------------------------------------------*/
		void M_Push(T pa_data)
		{
			ST_Node *lo_top, *new_node = m_pool->M_Alloc();
			new_node->data = pa_data;	new_node->link = nullptr;

			do
			{
				lo_top = m_top.node_address;
				new_node->link = (ST_Node*)lo_top;
			} while (InterlockedCompareExchange64((LONG64*)&m_top.node_address, (LONG64)new_node, (LONG64)lo_top) != (LONG64)lo_top);

			InterlockedIncrement(&m_stack_size);
		}

		/**-----------------------------------------------------------------------------------------------
		  * 스택 사용 카운트가 0이면 Crash
		  * ABA 상황이 발생할 수 있으므로 interlock128 함수 사용, lo_top 변수는 16바이트 정렬이 되어 있어야 함
		  *-----------------------------------------------------------------------------------------------*/
		T M_Pop()
		{
			if (InterlockedDecrement(&m_stack_size) < 0)
			{
				TCHAR lo_action[] = _TEXT("LFStack"), lo_server[] = _TEXT("Common");
				ST_Log lo_log({ "LFStack is Null" });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}

			__declspec(align(16)) ST_Top lo_top;
			T lo_return_value;

			do
			{
				lo_top.unique_index = m_top.unique_index;
				lo_top.node_address = m_top.node_address;

				lo_return_value = lo_top.node_address->data;
			} while (InterlockedCompareExchange128((LONG64*)&m_top, lo_top.unique_index + 1, (LONG64)(lo_top.node_address->link), (LONG64*)&lo_top) == 0);

			m_pool->M_Free(lo_top.node_address);
			return lo_return_value;
		}

		LONG M_GetAllocCount()
		{
			return m_pool->M_GetAllocCount();
		}

		LONG M_GetUseCount()
		{
			return m_stack_size;
		}
	};
}

#endif