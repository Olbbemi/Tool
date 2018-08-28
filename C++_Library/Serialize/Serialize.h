#ifndef Serialize_Info
#define Serialize_Info
#define SERIALIZE_BUFFER_SIZE 10000

#include <Windows.h>

#include "MemoryPool/MemoryPool.h"

namespace Olbbemi
{
	/**-------------------------------------------------------------------------------
      * 직렬화 버퍼 사용 시 스마트 포인터와 동일하게 객체의 소멸을 자동으로 처리하는 클래스
	  * 객체 생성 시 탬플릿 객체와 공용으로 사용할 카운트 할당
	  * 복사 생성자 및 이동 생성자를 통해 객체를 참조할 때마다 카운트 증가
	  * 소멸자 호출마다 카운트 감소 ( 카운트가 0이 되면 해당 직렬화 버퍼 반환 )
	  *-------------------------------------------------------------------------------*/
	template<class T>
	class C_Smart_Serialize
	{
	private:
		T* T_instance;
		volatile LONG* m_ref_count;

	public:
		C_Smart_Serialize()
		{
			T_instance = new T;
			m_ref_count = new LONG;
			*m_ref_count = 1;
		}

		C_Smart_Serialize(T* pa_instance)
		{
			T_instance = pa_instance;
			m_ref_count = new LONG;
			*m_ref_count = 1;
		}

		C_Smart_Serialize(const C_Smart_Serialize& pa_instance)
		{
			T_instance = pa_instance.T_instance;
			m_ref_count = pa_instance.m_ref_count;
			InterlockedIncrement(m_ref_count);
		}

		~C_Smart_Serialize()
		{
			LONG lo_inter_value = InterlockedDecrement(m_ref_count);
			if (lo_inter_value == 0)
			{
				delete T_instance;
				delete m_ref_count;
			}
		}

		T* operator -> ()
		{
			return T_instance;
		}

		C_Smart_Serialize& operator = (const C_Smart_Serialize& pa_instance)
		{
			LONG lo_inter_value = InterlockedDecrement(m_ref_count);
			if (lo_inter_value == 0)
			{
				delete T_instance;
				delete m_ref_count;
			}

			T_instance = pa_instance.T_instance;
			m_ref_count = pa_instance.m_ref_count;
			return *this;
		}
	};

	/**-------------------------------------------------------------------
	  * 직렬화 버퍼의 재사용 및 속도를 위해 TLS를 사용하는 MemoryPool 을 이용
	  * 해당 직렬화 버퍼를 참조할 때마다 S_AddReference 함수를 통해 해당 직렬화 버퍼의 사용 카운트를 증가
	  * 해당 직렬화 버퍼를 반환할 때마다 S_Free 함수를 호출 ( 사용 카운트 감소 후 0이되면 반환 )
	  *-------------------------------------------------------------------*/
	class C_Serialize
	{
	private:
		char* m_buffer_ptr;
		int m_front, m_rear, m_maximum_size;
		volatile LONG m_ref_count;

		static C_MemoryPoolTLS<C_Serialize> s_memory_pool;

	public:
		C_Serialize();
		~C_Serialize();

		static C_Serialize* S_Alloc();
		static void S_Free(C_Serialize* pa_serialQ);
		static void S_AddReference(C_Serialize* pa_serialQ);
		static LONG S_TLSAllocCount();
		static LONG S_TLSChunkCount();
		static LONG S_TLSNodeCount();


		void M_Enqueue(char *pa_src, const int pa_size);
		void M_Dequeue(char *pa_dest, const int pa_size);
		void M_Resize(const int pa_remain_size, const int pa_input_size);

		char* M_GetBufferPtr() const;
		int M_GetUsingSize() const;
		int M_GetUnusingSize() const;

		void M_MakeHeader(const char *pa_src, const int pa_size);
		void M_InputHeaderSize(const int pa_header_size);

		void M_MoveFront(const int pa_size);
		void M_MoveRear(const int pa_size);

		C_Serialize& operator <<(char* pa_data)
		{
			C_Serialize::M_Enqueue(pa_data, sizeof(pa_data));
			return *this;
		}

		C_Serialize& operator >>(char* pa_data)
		{
			C_Serialize::M_Dequeue(pa_data, sizeof(pa_data));
			return *this;
		}

		template<typename TYPE>
		C_Serialize& operator << (const TYPE &pa_data)
		{
			C_Serialize::M_Enqueue((char*)&pa_data, sizeof(pa_data));
			return *this;
		}

		template<typename TYPE>
		C_Serialize& operator >> (const TYPE &pa_data)
		{
			C_Serialize::M_Dequeue((char*)&pa_data, sizeof(pa_data));
			return *this;
		}
	};
}

#endif