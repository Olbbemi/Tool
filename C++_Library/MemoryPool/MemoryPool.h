#ifndef  MemoryPool_Info
#define  MemoryPool_Info

#define UNICODE
#define _UNICODE

#define MAX_ARRAY 200
#define CHECK_SUM 0x41
#define INITIAL_HEAP_SIZE 1024 * 1024

#include "Log/Log.h"

#include <new.h>
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>

#include <string>
#include <typeinfo>
using namespace std;

namespace Olbbemi
{
	template <class T>
	class C_MemoryPool
	{
	private:

		/**------------------------------------------------------------------------
		  * �޸� Ǯ���� ����� ���
		  * �� ��帶�� CheckSum �� �־� ��� ��ȯ �� ������ ������ ���� CheckSum Ȯ��
		  *------------------------------------------------------------------------*/
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

		/**---------------------------------------------------------------
		  * �޸� Ǯ���� ��带 �Ҵ��ϰ� ��ȯ �޴� ����� ������ ������ �̿�
		  * [0]: Node Address, [1]: Unique Index
		  *---------------------------------------------------------------*/
		struct ST_Top
		{
			volatile __int64 block_info[2];
		};

		bool m_is_placementnew;
		LONG m_use_count, m_alloc_count, m_max_alloc_count;
		HANDLE m_heap_handle;
		__declspec(align(16)) ST_Top m_pool_top;

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
			ST_Node* lo_next = nullptr;

			do
			{
				lo_top[1] = m_pool_top.block_info[1];
				lo_top[0] = m_pool_top.block_info[0];
				
				if (lo_top[0] == 0)
					return nullptr;

				lo_next = ((ST_Node*)lo_top[0])->next_node;
			} while (InterlockedCompareExchange128(m_pool_top.block_info, lo_top[1] + 1, (LONG64)lo_next, lo_top) == 0);

			if (m_is_placementnew == true)
				new((ST_Node*)lo_top[0]) ST_Node();

			return &(((ST_Node*)(lo_top[0]))->instance);
		}

	public:

		/**-----------------------------------------------------------------------------------------------------------
		  * �޸� Ǯ���� ����ϴ� ���� ����Ʈ ���� �ƴ� HeapCreate �� ���� ������ ���� �Ҵ� �޾� ���
		  * max_alloc_count �� 0�� �ƴϸ� �����ڿ��� �ش� ��ġ��ŭ ��带 �����Ͽ� ����
		  * placement_new �� false �̸� �� ��尡 ������ �� �ѹ��� ��� ������ ȣ��, �Ҹ��� �� �Ҹ��� ȣ��
		  * placement_new �� true �̸� Alloc �Լ� ȣ��� ������ ��� ������ ȣ��, Free �Լ� ȣ��� ������ ��� �Ҹ��� ȣ�� 
		  *-----------------------------------------------------------------------------------------------------------*/
		C_MemoryPool(int pa_max_alloc_count = 0, bool pa_is_placement_new = false)
		{
			m_heap_handle = HeapCreate(0, INITIAL_HEAP_SIZE, 0); 
			if (m_heap_handle == NULL)
			{
				TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
				ST_Log lo_log({ "Heap Create Error Code: " + to_string(GetLastError()) });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}

			m_max_alloc_count = pa_max_alloc_count;		m_is_placementnew = pa_is_placement_new;
			m_use_count = 0;							m_alloc_count = 0;
			m_pool_top.block_info[0] = 0;				m_pool_top.block_info[1] = 1;

			if (m_max_alloc_count != 0)
			{
				for (int i = 0; i < m_max_alloc_count; i++)
				{
					ST_Node *new_node = (ST_Node*)HeapAlloc(m_heap_handle, HEAP_ZERO_MEMORY, sizeof(ST_Node));
					if (m_is_placementnew == false)
						new(new_node) ST_Node();
				
					M_Push(new_node);
					InterlockedIncrement(&m_alloc_count);
				}
			}
		}

		/**----------------------------------------------------------------------------------
		  * �޸� Ǯ���� ����ϰ� �ִ� ��� ��带 HeapFree �Լ��� ���� ���� �� ��� ī��Ʈ ����
		  *  placement_new �� false �̸� �� ����� �Ҹ��� ȣ��
		  *----------------------------------------------------------------------------------*/
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

		/**---------------------------------------------------------------------------------------------------------
		  * �޸� Ǯ�� ��尡 �����Ѵٸ� �����ϴ� ��带 ��ȯ, �������� �ʴ´ٸ� HeapAlloc �Լ��� ���� ���� �Ҵ�޾� ��ȯ
		  * placement_new �� false �̸� HeapAlloc �Լ��� ���� ���� �Ҵ���� ���� ����� ������ ȣ��
		  * placement_new �� true �̸� M_Pop �Լ����� ��ȯ�ϱ��� ����� ������ ȣ��
		  *---------------------------------------------------------------------------------------------------------*/
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

		/**------------------------------------------------------------------------------------------------------
		  * ��ȯ���� �����͸� �̿��Ͽ� �����Ϳ����� ���� CheckSum Ȯ��, �����ߴٸ� �Ҵ��� ������Ÿ���� �ƴ� ���ڸ� ����
		  * placement_new �� true �̸� ��ȯ���� ������ �ش� ����� �Ҹ��� ȣ��
		  *------------------------------------------------------------------------------------------------------*/
		void M_Free(T* pData)
		{
			if (*((char*)pData + sizeof(T)) == CHECK_SUM)
			{
				if (m_is_placementnew == true)
					pData->~T();

				InterlockedDecrement(&m_use_count);
				M_Push((ST_Node*)((char*)pData - sizeof(T*)));
			}
			else
			{
				TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
				string lo_template_type = typeid(T).name(), lo_param_type = typeid(pData).name();

				ST_Log lo_log({ "Free Type Error -> TemplateType: " + lo_template_type + " ParamType: " + lo_param_type });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}
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

	template <class T>
	class C_MemoryPoolTLS
	{
	private:
		template <class T>
		class C_Chunk
		{
		private:

			/**---------------------------------------------------------------
			  * �� Chunk ���� ���� ��� ( �迭 ���·� ���� )
			  * data: TLSPool�� Alloc �Լ� ȣ�� �� ��ȯ�� �����͸� �����ϴ� ����
			  * alloc_index: TLSPool�� Free �Լ����� ������ �������� ���� ����
			  *---------------------------------------------------------------*/
			struct ST_Node
			{
				T data;
				C_Chunk* chunk_ptr;
			};

			volatile LONG m_free_count;
			ST_Node m_node[MAX_ARRAY];
			LONG m_index;

			template <class T> friend class C_MemoryPoolTLS;
		public:

			C_Chunk()
			{
				m_free_count = MAX_ARRAY;
				for (int i = 0; i < MAX_ARRAY; i++)
					m_node[i].chunk_ptr = this;
			}

			T* M_Chunk_Alloc(bool& pa_is_finish)
			{
				T* lo_return_value = &(m_node[m_index].data);

				m_index++;
				if (m_index == MAX_ARRAY)
					pa_is_finish = true;
				else
					pa_is_finish = false;

				return lo_return_value;
			}
		};

		DWORD m_tls_index;
		C_MemoryPool<C_Chunk<T>>* m_chunkpool;

	public:

		/**----------------------------------------------
		  * TlsAlloc �Լ��� ���� ��밡���� �ε����� ����
		  * �޸� Ǯ�� Chunk �� ����
		  *----------------------------------------------*/
		C_MemoryPoolTLS(bool pa_is_placement_new = false)
		{
			m_tls_index = TlsAlloc();
			if (m_tls_index == TLS_OUT_OF_INDEXES)
			{
				TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
				ST_Log lo_log({ "TlsAlloc Error Code: " + to_string(GetLastError()) });
				_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
			}

			m_chunkpool = new C_MemoryPool<C_Chunk<T>>(0, pa_is_placement_new);
		}

		/**-----------------------------------------------------------------------------------------------------------------------
		  * ���� �Ҵ���� TLS �� ��� �ִٸ� �޸� Ǯ�� �̿��Ͽ� Chunk �ּҸ� ���� ���� 
		  * TLS ���ο��� Chunk �ּҰ� ����Ǿ� �ְ� �� Chunk ���ο��� ��ȯ�� �����Ͱ� ����Ǿ� ����
		  * Alloc �Լ��� ȣ��� ������ Chunk ������ �����͸� ��ȯ�ϰ� �ش� Chunk���� �Ҵ��� �� �ִ� �����Ͱ� ��� ������ ��� TLS�� ��� 
		  *-----------------------------------------------------------------------------------------------------------------------*/
		T* M_Alloc()
		{
			bool lo_is_finish = false;
			void* lo_tls_ptr = nullptr;
			T* lo_return_value = nullptr;

			lo_tls_ptr = TlsGetValue(m_tls_index);
			if (lo_tls_ptr == nullptr)
			{
				int lo_error = GetLastError();
				if (lo_error != 0)
				{
					TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
					ST_Log lo_log({ "TlsGetValue Error Code: " + to_string(GetLastError()) });
					_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
				}

				lo_tls_ptr = (void*)(m_chunkpool->M_Alloc());
				BOOL lo_check = TlsSetValue(m_tls_index, lo_tls_ptr);
				if (lo_check == 0)
				{
					TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
					ST_Log lo_log({ "TlsSetValue Error Code: " + to_string(GetLastError()) });
					_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
				}
			}

			lo_return_value = &(((C_Chunk<T>*)lo_tls_ptr)->m_node[((C_Chunk<T>*)lo_tls_ptr)->m_index++].data);
			if (((C_Chunk<T>*)lo_tls_ptr)->m_index == MAX_ARRAY)
			{
				BOOL lo_check = TlsSetValue(m_tls_index, nullptr);
				if (lo_check == 0)
				{
					TCHAR lo_action[] = _TEXT("MemoryPool"), lo_server[] = _TEXT("NONE");
					ST_Log lo_log({ "TlsSetValue Error Code: " + to_string(GetLastError()) });
					_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
				}
			}

			return lo_return_value;
		}

		/**----------------------------------------------------------------------------------------------------------------
		  * ��ȯ ���� �����͸� �̿��Ͽ� �����Ϳ��� �� �� Chunk�� Free_count�� ������Ŵ
		  * �ش� Chunk ���� �Ҵ��� ��� ��尡 ��ȯ ( Free_count �� �迭 ũ��� ������ ��� ) �Ǹ� �ش� Chunk�� �޸� Ǯ�� ��ȯ
		  * (char*)pa_node + sizeof(T) ��ġ���� �ش� ��尡 ��ġ�ϴ� Chunk �����ּҸ� ����
		  *----------------------------------------------------------------------------------------------------------------*/
		void M_Free(T* pa_node)
		{
			LONG lo_interlock_value;
			C_Chunk<T>* lo_chunk_ptr = (C_Chunk<T>*)(*((LONG64*)((char*)pa_node + sizeof(T))));

			lo_interlock_value = InterlockedDecrement(&(lo_chunk_ptr->m_free_count));
			if (lo_interlock_value == 0)
			{
				lo_chunk_ptr->m_free_count = MAX_ARRAY;
				lo_chunk_ptr->m_index = 0;
				m_chunkpool->M_Free(lo_chunk_ptr);
			}
		}

		LONG M_UseChunkCount()
		{
			return m_chunkpool->M_GetUseCount();
		}

		LONG M_AllocCount()
		{
			return m_chunkpool->M_GetAllocCount();
		}
	};
}

#endif