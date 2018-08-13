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

		bool m_is_placementnew;
		LONG m_use_count, m_alloc_count;
		HANDLE m_heap_handle;
		__declspec(align(16)) ST_Top m_pool_top;

		void M_Push(ST_Node* pa_new_node)
		{
			__declspec(align(16)) __int64 lo_top[2];		

			do
			{
				lo_top[0] = m_pool_top.block_info[0];	
				lo_top[1] = m_pool_top.block_info[1];
				
				pa_new_node->next_node = (ST_Node*)lo_top[0];
			} while (InterlockedCompareExchange128(m_pool_top.block_info, lo_top[1] + 1, (LONG64)pa_new_node, lo_top) == 0);
		}

		T* M_Pop()
		{
			__declspec(align(16)) __int64 lo_top[2];		
			ST_Node* lo_next = nullptr;

			do
			{
				lo_top[0] = m_pool_top.block_info[0];	
				lo_top[1] = m_pool_top.block_info[1];
			
				if(lo_top[0] == 0)
					return nullptr;
				
				lo_next = ((ST_Node*)lo_top[0])->next_node;
			} while (InterlockedCompareExchange128(m_pool_top.block_info, lo_top[1] + 1, (LONG64)lo_next, lo_top) == 0);

			if (m_is_placementnew == true)
				new((ST_Node*)lo_top[0]) ST_Node();

			return &(((ST_Node*)(lo_top[0]))->instance);
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
			m_use_count = 0;					m_alloc_count = 0;
			m_pool_top.block_info[0] = 0;	m_pool_top.block_info[1] = 1;
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

		T* M_Alloc()
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

				return_value = M_Pop();
				if (return_value != nullptr)
					break;
			}

			InterlockedIncrement(&m_use_count);
			return return_value;
		}

		bool M_Free(T* pData)
		{
			if (*((char*)pData + sizeof(T)) == CHECK_SUM)
			{
				if (m_is_placementnew == true)
					pData->~T();

				InterlockedDecrement(&m_use_count);
				M_Push((ST_Node*)((char*)pData - sizeof(T*)));
				return true;
			}
			else
				return false;
		}
	
		int	M_GetAllocCount() const // �޸�Ǯ ������ ��ü ����
		{
			return m_alloc_count;
		}
	
		int	M_GetUseCount() const // ���� ������� �� ����
		{ 
			return m_use_count;
		}
	};
}
#endif