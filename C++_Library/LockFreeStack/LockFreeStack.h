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
		  * ������ ���ÿ��� ���ø����� ���޵� �ڷ����� ������ �����͸� ������ ��� ����ü
		  * �ܹ��� ��ũ�� ����Ʈ�����̹Ƿ� ���� ��带 ������ �����͸� ����
		  *--------------------------------------------------------------------------*/
		struct ST_Node
		{
			T data;
			ST_Node* link;
		};

		/**-------------------------------------------------------------------
		  * ������ ���ÿ��� ��� ������ ������ Front �� Rear ������ ����� ����ü
		  * ������ ������ ABA ������ �ذ��ϱ� ���� 128bit ���
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
		  * ���� ������ ������ �ִ� ��� ��带 �޸� Ǯ�� ��ȯ �� ��� ī��Ʈ ����
		  * �޸� Ǯ ��ü ����
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
		  * ������ ���ÿ��� Push �Լ��� ABA ��Ȳ�� �������� �����Ƿ� interlock64 �Լ� ���
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
		  * ���� ��� ī��Ʈ�� 0�̸� Crash
		  * ABA ��Ȳ�� �߻��� �� �����Ƿ� interlock128 �Լ� ���, lo_top ������ 16����Ʈ ������ �Ǿ� �־�� ��
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