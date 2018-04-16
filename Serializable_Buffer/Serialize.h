#ifndef Serialize_Info
#define Serialize_Info

class SERIALIZE
{
private:
	char *buffer_ptr;
	int front, rear, buffer_size;

	void resize(int size);

	void Enqueue(char *src, int size);
	bool Dequeue(char *dest, int size);
public:
	SERIALIZE();
	SERIALIZE(int size);

	int GetUsingData();
	int GetUnusingData();

	char* GetBufferPtr();

	void MoveFront(int size);
	void MoveRear(int size);

	template<typename TYPE>
	SERIALIZE& operator << (const TYPE &data)
	{
		SERIALIZE::Enqueue((char*)&data, sizeof(TYPE));
		return *this;
	}

	template<typename TYPE>
	SERIALIZE& operator >> (const TYPE &data)
	{
		SERIALIZE::Dequeue((char*)&data, sizeof(TYPE));
		return *this;
	}
};

#endif