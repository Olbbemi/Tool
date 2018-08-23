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

namespace Olbbemi
{
   /*
	* 메모리 풀은 기본 힙을 사용하지 않음 ( HeapCreate, HeapAlloc, HeapFree, HeapDestroy 이용 )
	* placement new : new 연산자에서 제공하는 3가지 성질 [ 1.동적할당, 2.객체에 대하여 생성자 호출, 3.가상테이블 생성 ] 중 2, 3번을 수행하는 연산자
	* MemoryPool 의 생성자 매개변수에 p_is_placement_new 가 false : NODE 생성 시 최초에 한번만 생성자호출 및 소멸자 호출 [ malloc 으로 할당하고 placement_new 호출, 소멸자 직접 호출 ]
	* MemoryPool 의 생성자 매개변수에 p_is_placement_new 가 true :  NODE 생성 시에는 malloc으로 할당, 외부에서 Alloc 및 Free 함수를 호출할 때마다 placement_new 이용하여 생성자 및 소멸자 호출 ]
	*/

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
	
		int	M_GetAllocCount() const // 메모리풀 내부의 전체 개수
		{
			return m_alloc_count;
		}
	
		int	M_GetUseCount() const // 현재 사용중인 블럭 개수
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
			struct ST_Node
			{
				T data;
				WORD alloc_index;
				C_Chunk<T>* chunk_ptr;
			};

			volatile LONG m_free_count;
			ST_Node m_node[MAX_ARRAY];
			LONG m_index;

		public:
			C_Chunk()
			{
				m_free_count = MAX_ARRAY;
				for (int i = 0; i < MAX_ARRAY; i++)
				{
					m_node[i].alloc_index = i;
					m_node[i].chunk_ptr = this;
				}
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

			lo_return_value = ((C_Chunk<T>*)lo_tls_ptr)->M_Chunk_Alloc(lo_is_finish);
			if (lo_is_finish == true)
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

		void M_Free(T* pa_node)
		{
			/*
			 * (char*)pa_node + 16 -> 해당 노드의 배열 인덱스 번호
			 * (char*)pa_node + 24 -> 해당 노드가 속한 Chunk 의 시작 주소
			 * (char*)pa_node + 32 -> 각 Chunk의 RefCount
			 */

			LONG lo_interlock_value = InterlockedDecrement((LONG*)((char*)pa_node - (*((WORD*)((char*)pa_node + 16)) * 32 + 8)));
			if (lo_interlock_value == 0)
			{
				*((LONG64*)((char*)pa_node + 32)) = 0;
				*(LONG64*)((LONG*)((char*)pa_node - (*((WORD*)((char*)pa_node + 16)) * 32 + 8))) = MAX_ARRAY;
				m_chunkpool->M_Free((C_Chunk<T>*)(*((LONG64*)((char*)pa_node + 24))));
			}
		}

		int M_UseChunkCount()
		{
			return m_chunkpool->M_GetUseCount();
		}

		int M_AllocCount()
		{
			return m_chunkpool->M_GetAllocCount();
		}
	};
}

#endif