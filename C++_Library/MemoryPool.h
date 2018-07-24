#ifndef  MemoryPool_Info
#define  MemoryPool_Info
#define CHECK_SUM 0x41

#define UNICODE
#define _UNICODE

#include "Log.h"

#include <new.h>
#include <tchar.h>
#include <stdlib.h>
#include <typeinfo>
#include <Windows.h>

/*
 * placement new : new 연산자에서 제공하는 3가지 성질 [ 1.동적할당, 2.객체에 대하여 생성자 호출, 3.가상테이블 생성 ] 중 2, 3번을 수행하는 연산자
 * MemoryPool 의 생성자 매개변수에 p_is_placement_new 가 false : NODE 생성 시 최초에 한번만 생성자호출 및 소멸자 호출 [ malloc 으로 할당하고 placement_new 호출, 소멸자 직접 호출 ]
 * MemoryPool 의 생성자 매개변수에 p_is_placement_new 가 true :  NODE 생성 시에는 malloc으로 할당, 외부에서 Alloc 및 Free 함수를 호출할 때마다 placement_new 이용하여 생성자 및 소멸자 호출 ]
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
		
		bool m_is_placementnew;
		int m_max_count;

	public:
		MemoryPool(int p_block_max_count, bool p_is_placement_new = false)
		{
			InitializeSRWLock(&m_pool_srw_lock);

			m_max_count = p_block_max_count;	m_is_placementnew = p_is_placement_new;
			m_use_count = 0;					m_alloc_count = 1;
			m_wrong_free = 0;

			m_pool_top = (NODE*)malloc(sizeof(NODE));
			if (p_is_placement_new == false)
				new(m_pool_top) NODE();
		}

		virtual	~MemoryPool()
		{
			NODE* garbage;
			while (m_pool_top != nullptr)
			{
				garbage = m_pool_top;
				if (m_is_placementnew == false)
					garbage->s_data.~DATA();

				m_alloc_count--;
				m_pool_top = m_pool_top->s_next_block;
				free(garbage);
			}
		}

		DATA* Alloc()
		{
			if (m_pool_top->s_next_block == nullptr)
			{
				NODE *new_node = (NODE*)malloc(sizeof(NODE));
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

			return return_value;
		}

		bool Free(DATA *pData)
		{
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
				TCHAR buffer[100], object[50], param[50];
				ZeroMemory(buffer, 100);  ZeroMemory(object, 50);	ZeroMemory(param, 50);

				MultiByteToWideChar(CP_UTF8, 0, typeid(DATA).name(), strlen(typeid(DATA).name()), object, 50);
				MultiByteToWideChar(CP_UTF8, 0, typeid(pData).name(), strlen(typeid(pData).name()), param, 50);
	
				_stprintf_s(buffer, 100, _TEXT("MemoryPool Type: [%s], Free_Value Type: [%s]"), object, param);
				LOG::PrintLog(__LINE__, LOG_LEVEL_ERROR, 1, buffer);

				m_wrong_free++;
				return false;
			}
		}

		// 메모리풀 내부의 전체 개수
		int	GetAllocCount()
		{
			return m_alloc_count;
		}

		// 현재 사용중인 블럭 개수
		int	GetUseCount() 
		{ 
			return m_use_count;
		}

		void Pool_Lock()
		{
			AcquireSRWLockExclusive(&m_pool_srw_lock);
		}

		void Pool_Unlock()
		{
			ReleaseSRWLockExclusive(&m_pool_srw_lock);
		}
	};
}
#endif