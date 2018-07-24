#ifndef Serialize_Info
#define Serialize_Info

#define SERIALIZE_BUFFER_SIZE 10000

/*
 * 직렬화 버퍼는 항상 메모리풀에서만 사용 [ Option: placement new = false ] -> 버퍼의 생성과 삭제는 초기 생성자 호출 한번과 마지막 소멸자 호출 한번에서만 발생
 * 현재 입력되는 직렬화 버퍼의 공간이 부족하면 자동으로 Resize
 * 메모리풀에서 가져오면 항상 InputHeaderSize 함수로 초기화
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