#ifndef Serialize_Info
#define Serialize_Info

#define SERIALIZE_BUFFER_SIZE 10000

/*
 * ����ȭ ���۴� �׻� �޸�Ǯ������ ��� [ Option: placement new = false ] -> ������ ������ ������ �ʱ� ������ ȣ�� �ѹ��� ������ �Ҹ��� ȣ�� �ѹ������� �߻�
 * ���� �ԷµǴ� ����ȭ ������ ������ �����ϸ� �ڵ����� Resize
 * �޸�Ǯ���� �������� �׻� InputHeaderSize �Լ��� �ʱ�ȭ
 */

namespace Olbbemi
{
	class SERIALIZE
	{
	private:
		char* m_buffer_ptr;
		int m_front, m_rear, m_total_length;

	public:
		SERIALIZE();
		~SERIALIZE();
		void Enqueue(char *p_src, const int p_size);
		void Dequeue(char *p_dest, const int p_size);

		void Resize(int p_remain_size, int p_input_size);
		int GetUsingSize();
		int GetUnusingSize();

		char* GetBufferPtr();
		void MakeHeader(const char *p_src, const int p_size);
		void InputHeaderSize(int p_header_size);

		void MoveFront(const int p_size);
		void MoveRear(const int p_size);

		SERIALIZE& operator <<(char* p_data)
		{
			SERIALIZE::Enqueue(p_data, sizeof(p_data));
			return *this;
		}

		SERIALIZE& operator >>(char* p_data)
		{
			SERIALIZE::Dequeue(p_data, sizeof(p_data));
			return *this;
		}

		template<typename TYPE>
		SERIALIZE& operator << (const TYPE &p_data)
		{
			SERIALIZE::Enqueue((char*)&p_data, sizeof(p_data));
			return *this;
		}

		template<typename TYPE>
		SERIALIZE& operator >> (const TYPE &p_data)
		{
			SERIALIZE::Dequeue((char*)&p_data, sizeof(p_data));
			return *this;
		}
	};
}

#endif