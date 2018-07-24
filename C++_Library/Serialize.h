#ifndef Serialize_Info
#define Serialize_Info
#define SERIALIZE_BUFFER_SIZE 10000

namespace Olbbemi
{
	class SERIALIZE
	{
	private:
		char m_buffer_ptr[SERIALIZE_BUFFER_SIZE];
		int m_front, m_rear;

	public:
		SERIALIZE(int p_header_size);
		~SERIALIZE();

		void Enqueue(char *p_src, const int p_size);
		void Dequeue(char *p_dest, const int p_size);

		int GetUsingSize();
		int GetUnusingSize();

		char* GetBufferPtr();
		void MakeHeader(const char *p_src, const int p_size);

		void MoveFront(const int p_size);
		void MoveRear(const int p_size);

		// 템플릿 특수화를 이용하여 배열을 넣을 수 있도록 처리

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