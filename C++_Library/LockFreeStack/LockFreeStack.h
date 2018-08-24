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
			volatile LONG64 block_info[2];
		};

		volatile LONG m_use_count;
		__declspec(align(16)) ST_Top m_top;
		C_MemoryPool<ST_Node> *m_pool;

	public:

		C_LFStack()
		{
			m_pool = new C_MemoryPool<ST_Node>(0, false);

			m_top.block_info[0] = 0;
			m_top.block_info[1] = 1;
		}

		/**---------------------------------------------------------------------
		  * ���� ������ ������ �ִ� ��� ��带 �޸� Ǯ�� ��ȯ �� ��� ī��Ʈ ����
		  * �޸� Ǯ ��ü ����
		  *---------------------------------------------------------------------*/
		~C_LFStack()
		{
			ST_Node* lo_garbage;
			while (m_top.block_info[0] != 0)
			{
				m_use_count--;
				lo_garbage = (ST_Node*)m_top.block_info[0];
				m_top.block_info[0] = (LONG64)(((ST_Node*)m_top.block_info[0])->link);
				m_pool->M_Free(lo_garbage);
			}

			delete m_pool;
		}

		/**----------------------------------------------------------------------------
		  * ������ ���ÿ��� Push �Լ��� ABA ��Ȳ�� �������� �����Ƿ� interlock64 �Լ� ���
		  *----------------------------------------------------------------------------*/
		void M_Push(T pa_data)
		{
			LONG64 lo_top;
			ST_Node* new_node = m_pool->M_Alloc();
			new_node->data = pa_data;	new_node->link = nullptr;

			do
			{
				lo_top = m_top.block_info[0];
				new_node->link = (ST_Node*)lo_top;
			} while (InterlockedCompareExchange64((LONG64*)&m_top.block_info, (LONG64)new_node, lo_top) != lo_top);

			InterlockedIncrement(&m_use_count);
		}

		/**-----------------------------------------------------------------------------------------------
		  * ���� ��� ī��Ʈ�� 0�̸� Crash
		  * ABA ��Ȳ�� �߻��� �� �����Ƿ� interlock128 �Լ� ���, lo_top ������ 16����Ʈ ������ �Ǿ� �־�� ��
		  *-----------------------------------------------------------------------------------------------*/
		T M_Pop()
		{
			if (InterlockedDecrement(&m_use_count) < 0)
			{
				TCHAR lo_action[] = _TEXT("LFStack"), lo_server[] = _TEXT("NONE");
				ST_Log lo_log({ "LFStack is Null" });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}

			__declspec(align(16)) LONG64 lo_top[2];
			T lo_return_value;
			ST_Node* lo_next = nullptr;

			do
			{
				lo_top[1] = m_top.block_info[1];
				lo_top[0] = m_top.block_info[0];
				
				lo_next = ((ST_Node*)lo_top[0])->link;
				lo_return_value = ((ST_Node*)(lo_top[0]))->data;
			} while (InterlockedCompareExchange128(m_top.block_info, lo_top[1] + 1, (LONG64)lo_next, lo_top) == 0);

			m_pool->M_Free((ST_Node*)lo_top[0]);
			return lo_return_value;
		}

		LONG M_GetAllocCount()
		{
			return m_pool->M_GetAllocCount();
		}
	};
}

#endif