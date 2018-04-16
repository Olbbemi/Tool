#ifndef Ring_Buffer_Info
#define Ring_Buffer_Info

class RINGBUFFER
{
private:
	char *buffer;
	int front, rear;

public:
	RINGBUFFER();
	~RINGBUFFER();

	bool Enqueue(char *data, int size);
	bool Dequeue(char *dest, int size, int &return_size);
	bool Peek(char *dest, int size, int &return_size);

	int GetUseSize();
	int GetUnuseSize();

	char* GetFrontPtr();
	char* GetRearPtr();
	
	void MoveFront(int size);
	void MoveRear(int size);

	void ClearBuffer();
		
	int LinearRemainFrontSize();
	int LinearRemainRearSize();
};

#endif
