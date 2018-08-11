#ifndef  MemoryPool_Info
#define  MemoryPool_Info

#define UNICODE
#define _UNICODE
#define CHECK_SUM 0x41
#define INITIAL_HEAP_SIZE 1024 * 1024

#include "Log/Log.h"

#include <new.h>
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>

/*
 * �޸� Ǯ�� �⺻ ���� ������� ���� ( HeapCreate, HeapAlloc, HeapFree, HeapDestroy �̿� )
 * placement new : new �����ڿ��� �����ϴ� 3���� ���� [ 1.�����Ҵ�, 2.��ü�� ���Ͽ� ������ ȣ��, 3.�������̺� ���� ] �� 2, 3���� �����ϴ� ������
 * MemoryPool �� ������ �Ű������� p_is_placement_new �� false : NODE ���� �� ���ʿ� �ѹ��� ������ȣ�� �� �Ҹ��� ȣ�� [ malloc ���� �Ҵ��ϰ� placement_new ȣ��, �Ҹ��� ���� ȣ�� ]
 * MemoryPool �� ������ �Ű������� p_is_placement_new �� true :  NODE ���� �ÿ��� malloc���� �Ҵ�, �ܺο��� Alloc �� Free �Լ��� ȣ���� ������ placement_new �̿��Ͽ� ������ �� �Ҹ��� ȣ�� ]
 */

namespace Olbbemi
{
	template <class T>
	class C_MemoryPool
	{
	private:
		struct ST_Node
		{
			ST_Node* next_node;
			T instance;
			BYTE checksum;

			ST_Node()
			{
				next_node = nullptr;
				checksum = CHECK_SUM;
			}
		};
		
		struct ST_Top
		{
			volatile __int64 block_info[2];
		};

		LONG m_use_count, m_alloc_count;
		HANDLE m_heap_handle;
		__declspec(align(16)) ST_Top m_pool_top;

		bool m_is_placementnew;
		__int64 m_unique_index;

		void M_Push(ST_Node* pa_new_node)
		{
			__int64 lo_top;

			do
			{
				lo_top = m_pool_top.block_info[0];
				pa_new_node->next_node = (ST_Node*)lo_top;
			} while (InterlockedCompareExchange64((LONG64*)&m_pool_top.block_info[0], (LONG64)pa_new_node, lo_top) != lo_top);
		}

		T* M_Pop()
		{
			__declspec(align(16)) __int64 lo_top[2];
			T* lo_return_value = nullptr;
			LONG64 lo_value = InterlockedIncrement64(&m_unique_index);
			ST_Node* lo_next = nullptr;

			do
			{
				lo_top[0] = m_pool_top.block_info[0];
				lo_top[1] = m_pool_top.block_info[1];

				if(lo_top[0] == 0)
					return nullptr;
				
				lo_next = ((ST_Node*)lo_top[0])->next_node;
				lo_return_value = &(((ST_Node*)(lo_top[0]))->instance);

			} while (InterlockedCompareExchange128(m_pool_top.block_info, lo_value, (LONG64)lo_next, lo_top) == 0);

			return lo_return_value;
		}

	public:
		C_MemoryPool(bool p_is_placement_new = false)
		{
			m_heap_handle = HeapCreate(0, INITIAL_HEAP_SIZE, 0); 
			if (m_heap_handle == NULL)
			{
				TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
				ST_Log lo_log({ "Heap Create Error Code: " + to_string(GetLastError()) });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}

			m_is_placementnew = p_is_placement_new;
			m_use_count = 0;					m_alloc_count = 0;		m_unique_index = 1;
			m_pool_top.block_info[0] = 0;	m_pool_top.block_info[1] = 0;
		}

		virtual	~C_MemoryPool()
		{
			bool lo_destroy_check;
			int lo_free_check;
			ST_Node* lo_garbage;

			while (m_pool_top.block_info[0] != 0)
			{
				lo_garbage = (ST_Node*)m_pool_top.block_info[0];
				if (m_is_placementnew == false)
					lo_garbage->instance.~T();

				m_pool_top.block_info[0] = (LONG64)(((ST_Node*)m_pool_top.block_info[0])->next_node);
				InterlockedDecrement(&m_alloc_count);

				lo_free_check = HeapFree(m_heap_handle, 0, lo_garbage);
				if (lo_free_check == 0)
				{
					TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
					ST_Log lo_log({ "Heap Free Error Code: " + to_string(GetLastError()) });
					_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
				}
			}

			lo_destroy_check = HeapDestroy(m_heap_handle);
			if (lo_destroy_check == false)
			{
				TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
				ST_Log lo_log({ "Heap Destroy Error Code: " + to_string(GetLastError()) });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}
		}

		T* Alloc()
		{
			T* return_value;
			while (1)
			{
				if (m_pool_top.block_info[0] == 0)
				{
					ST_Node *new_node = (ST_Node*)HeapAlloc(m_heap_handle, HEAP_ZERO_MEMORY, sizeof(ST_Node));
					if (m_is_placementnew == false)
						new(new_node) ST_Node();

					InterlockedIncrement(&m_use_count);
					InterlockedIncrement(&m_alloc_count);
					return &new_node->instance;
				}

				if (m_is_placementnew == true)
					new((ST_Node*)m_pool_top.block_info[0]) ST_Node();

				return_value = M_Pop();
				if (return_value != nullptr)
					break;
			}

			InterlockedIncrement(&m_use_count);
			return return_value;
		}

		bool Free(T* pData)
		{
			if (*((char*)pData + sizeof(T)) == CHECK_SUM)
			{
				if (m_is_placementnew == true)
					pData->~T();

				M_Push((ST_Node*)((char*)pData - sizeof(T*)));
				InterlockedDecrement(&m_use_count);			
				return true;
			}
			else
				return false;
		}
	
		int	GetAllocCount() const // �޸�Ǯ ������ ��ü ����
		{
			return m_alloc_count;
		}
	
		int	GetUseCount() const // ���� ������� �� ����
		{ 
			return m_use_count;
		}
	};
}
#endif