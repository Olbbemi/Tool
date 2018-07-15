#ifndef Serialize_Info
#define Serialize_Info

#define SERIALIZE_BUFFER_SIZE 10000

class SERIALIZE
{
private:
	char *m_buffer_ptr;
	int m_front, m_rear;

	//void resize(int size); -> 추후 메모리풀에 적용할 때 고려하여 작성할 것!!!

public:
	SERIALIZE(int p_header_size, int p_size = SERIALIZE_BUFFER_SIZE);
	~SERIALIZE();

	void Enqueue(char *p_src, const int p_size);
	void Dequeue(char *p_dest, const int p_size);

	int GetUsingSize();
	int GetUnusingSize();

	char* GetBufferPtr();
	void MakeHeader(const char *p_src, const int p_size);

	void MoveFront(const int p_size);
	void MoveRear(const int p_size);

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

#endif
