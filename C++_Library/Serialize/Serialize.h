#ifndef Serialize_Info
#define Serialize_Info
#define SERIALIZE_BUFFER_SIZE 10000

#include <Windows.h>

#include "MemoryPool/MemoryPool.h"

namespace Olbbemi
{
	/**-------------------------------------------------------------------------------
      * ����ȭ ���� ��� �� ����Ʈ �����Ϳ� �����ϰ� ��ü�� �Ҹ��� �ڵ����� ó���ϴ� Ŭ����
	  * ��ü ���� �� ���ø� ��ü�� �������� ����� ī��Ʈ �Ҵ�
	  * ���� ������ �� �̵� �����ڸ� ���� ��ü�� ������ ������ ī��Ʈ ����
	  * �Ҹ��� ȣ�⸶�� ī��Ʈ ���� ( ī��Ʈ�� 0�� �Ǹ� �ش� ����ȭ ���� ��ȯ )
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
	  * ����ȭ ������ ���� �� �ӵ��� ���� TLS�� ����ϴ� MemoryPool �� �̿�
	  * �ش� ����ȭ ���۸� ������ ������ S_AddReference �Լ��� ���� �ش� ����ȭ ������ ��� ī��Ʈ�� ����
	  * �ش� ����ȭ ���۸� ��ȯ�� ������ S_Free �Լ��� ȣ�� ( ��� ī��Ʈ ���� �� 0�̵Ǹ� ��ȯ )
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