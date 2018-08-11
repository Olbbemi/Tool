#ifndef  MemoryPool_Info
#define  MemoryPool_Info

#define UNICODE
#define _UNICODE
#define CHECK_SUM 0x41
#define INITIAL_HEAP_SIZE 1024 * 1024

#include "Log.h"

#include <new.h>
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>

#include <string>
#include <typeinfo>
#include <initializer_list>

using namespace std;

/*
 * �޸� Ǯ�� �⺻ ���� ������� ���� ( HeapCreate, HeapAlloc, HeapFree, HeapDestroy �̿� )
 * placement new : new �����ڿ��� �����ϴ� 3���� ���� [ 1.�����Ҵ�, 2.��ü�� ���Ͽ� ������ ȣ��, 3.�������̺� ���� ] �� 2, 3���� �����ϴ� ������
 * MemoryPool �� ������ �Ű������� p_is_placement_new �� false : NODE ���� �� ���ʿ� �ѹ��� ������ȣ�� �� �Ҹ��� ȣ�� [ malloc ���� �Ҵ��ϰ� placement_new ȣ��, �Ҹ��� ���� ȣ�� ]
 * MemoryPool �� ������ �Ű������� p_is_placement_new �� true :  NODE ���� �ÿ��� malloc���� �Ҵ�, �ܺο��� Alloc �� Free �Լ��� ȣ���� ������ placement_new �̿��Ͽ� ������ �� �Ҹ��� ȣ�� ]
 */

namespace Olbbemi
{
	template <class DATA>
	class MemoryPool
	{
	private:
		struct NODE
		{
			NODE* s_next_block;
			DATA s_data;
			BYTE s_checksum;

			NODE()
			{
				s_next_block = nullptr;
				s_checksum = CHECK_SUM;
			}
		};
		
		WORD m_use_count, m_alloc_count, m_wrong_free;
		NODE* m_pool_top;
		SRWLOCK m_pool_srw_lock;
		HANDLE m_heap_handle;

		bool m_is_placementnew;
		int m_max_count;

	public:
		MemoryPool(int p_block_max_count, bool p_is_placement_new = false)
		{
			InitializeSRWLock(&m_pool_srw_lock);
			
			/*
			 * Alloc �� Free �Լ��� SRWLock ����ϹǷ� Heap ������ Lock ���ʿ�
			 * ���������� ���� 0: �ʱ� ������ �޸� ũ�⸦ �ʰ��� �� �ڵ����� ����
			 */
			m_heap_handle = HeapCreate(HEAP_NO_SERIALIZE, INITIAL_HEAP_SIZE, 0); if (m_heap_handle == NULL)
			{
				TCHAR action[] = _TEXT("MemoryPool"), server[] = _TEXT("NONE");
				initializer_list<string> str = { "Heap Create Error Code: " + to_string(GetLastError()) };
				LOG::PrintLog(__LINE__, LOG_LEVEL_ERROR, action, server, str);
				throw;
			}

			m_max_count = p_block_max_count;	m_is_placementnew = p_is_placement_new;
			m_use_count = 0;					m_alloc_count = 1;
			m_wrong_free = 0;

			m_pool_top = (NODE*)HeapAlloc(m_heap_handle, HEAP_ZERO_MEMORY, sizeof(NODE));
			if (m_pool_top == NULL)
			{
				TCHAR action[] = _TEXT("MemoryPool"), server[] = _TEXT("NONE");
				initializer_list<string> str = { "Heap Alloc Error Code: " + to_string(GetLastError()) };
				LOG::PrintLog(__LINE__, LOG_LEVEL_ERROR, action, server, str);
				throw;
			}

			if (p_is_placement_new == false)
				new(m_pool_top) NODE();
		}

		virtual	~MemoryPool()
		{
			bool destroy_check;
			int free_check;
			NODE* garbage;

			while (m_pool_top != nullptr)
			{
				garbage = m_pool_top;
				if (m_is_placementnew == false)
					garbage->s_data.~DATA();

				m_alloc_count--;
				m_pool_top = m_pool_top->s_next_block;
				free_check = HeapFree(m_heap_handle, 0, garbage);

				if (free_check == 0)
				{
					TCHAR action[] = _TEXT("MemoryPool"), server[] = _TEXT("NONE");
					initializer_list<string> str = { "Heap Free Error Code: " + to_string(GetLastError()) };
					LOG::PrintLog(__LINE__, LOG_LEVEL_ERROR, action, server, str);
					throw;
				}
			}

			destroy_check = HeapDestroy(m_heap_handle);
			if (destroy_check == false)
			{
				TCHAR action[] = _TEXT("MemoryPool"), server[] = _TEXT("NONE");
				initializer_list<string> str = { "Heap Destroy Error Code: " + to_string(GetLastError()) };
				LOG::PrintLog(__LINE__, LOG_LEVEL_ERROR, action, server, str);
				throw;
			}
		}

		DATA* Alloc()
		{
			AcquireSRWLockExclusive(&m_pool_srw_lock);

			if (m_pool_top->s_next_block == nullptr)
			{
				NODE *new_node = (NODE*)HeapAlloc(m_heap_handle, HEAP_ZERO_MEMORY, sizeof(NODE));
				if (m_is_placementnew == false)
					new(new_node) NODE();

				m_pool_top->s_next_block = new_node;
				m_alloc_count++;
			}
			
			if (m_is_placementnew == true)
				new(m_pool_top->s_next_block) DATA();
			
			DATA* return_value = &m_pool_top->s_data;
			m_pool_top = m_pool_top->s_next_block;
			m_use_count++;

			ReleaseSRWLockExclusive(&m_pool_srw_lock);
			return return_value;
		}

		bool Free(DATA* pData)
		{
			AcquireSRWLockExclusive(&m_pool_srw_lock);

			if (*((char*)pData + sizeof(DATA)) == CHECK_SUM)
			{
				if (m_is_placementnew == true)
					pData->~DATA();

				m_pool_top = (NODE*)((char*)pData - sizeof(DATA*));
				m_use_count--;
				return true;
			}
			else
			{
				TCHAR action[] = _TEXT("MemoryPool"), server[] = _TEXT("NONE");
				string object = typeid(DATA).name(), param = typeid(pData).name();

				initializer_list<string> str = { "PoolAlloc Type: [" + object + "], Free_Value Type: [" + param + "]" };
				LOG::PrintLog(__LINE__, LOG_LEVEL_ERROR, action, server, str);

				m_wrong_free++;
				return false;
			}

			ReleaseSRWLockExclusive(&m_pool_srw_lock);
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