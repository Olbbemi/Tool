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
	bool Dequeue(char *dest, int dest_size, int size);
	bool Peek(char *dest, int dest_size, int size, int *return_size);

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

#define BUFSIZE 16

#endif
