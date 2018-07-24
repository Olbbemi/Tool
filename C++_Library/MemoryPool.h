#ifndef  MemoryPool_Info
#define  MemoryPool_Info
#define CHECK_SUM 0x41

#include <new.h>
#include <stdlib.h>
#include <Windows.h>

/*
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
		
		WORD m_use_count, m_alloc_count;
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

				free(garbage);

				m_pool_top = m_pool_top->s_next_block;
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
				// log || dump
				return false;
			}
		}

		// �޸�Ǯ ������ ��ü ����
		int	GetAllocCount()
		{
			return m_alloc_count;
		}

		// ���� ������� �� ����
		int	GetUseCount() 
		{ 
			return m_use_count;
		}

		void Lock()
		{
			AcquireSRWLockExclusive(&m_pool_srw_lock);
		}

		void Unlock()
		{
			ReleaseSRWLockExclusive(&m_pool_srw_lock);
		}
	};
}
#endif