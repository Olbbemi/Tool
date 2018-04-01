#include "Precompile.h"
#include "RingBuffer.h"

#include <string.h>

RINGBUFFER::RINGBUFFER()
{
	buffer = new char[BUFSIZE];
	front = rear = 0;
}

RINGBUFFER::~RINGBUFFER()
{
	delete[] buffer;
}

int RINGBUFFER::GetUseSize()
{
	if (front > rear)
		return BUFSIZE - (front - rear);	//(BUFSIZE - 1) - (front - rear) + 1; 
	else
		return rear - front;
}

int RINGBUFFER::GetUnuseSize()
{
	if (front > rear)
		return front - rear - 1;
	else
		return (BUFSIZE - 1) - (rear - front);
}

bool RINGBUFFER::Enqueue(char *data, int size)
{
	int gap, unusing_data = GetUnuseSize();

	if ((rear + 1) % BUFSIZE == front || unusing_data < size)
		return false;

	rear++;
	if (rear == BUFSIZE)
		rear = 0;

	gap = BUFSIZE - rear;
	if (gap < size)
	{
		memcpy_s(buffer + rear, BUFSIZE, data, gap);

		memcpy_s(buffer, BUFSIZE, data + gap, size - gap);
		rear = size - gap - 1;
	}
	else
	{
		memcpy_s(buffer + rear, BUFSIZE, data, size);

		rear = (rear + size - 1);
		if (rear >= BUFSIZE)
			rear -= BUFSIZE;
	}

	return true;
}

bool RINGBUFFER::Dequeue(char *dest, int dest_size, int size)
{
	int gap, using_data = GetUseSize();

	if (rear == front || using_data < size)
		return false;

	front++;
	if (front == BUFSIZE)
		front = 0;

	gap = BUFSIZE - front;
	if (gap < size)
	{
		memcpy_s(dest, dest_size, buffer + front, gap);

		memcpy_s(dest + gap, dest_size, buffer, size - gap);
		front = size - gap - 1;
	}
	else
	{
		memcpy_s(dest, dest_size, buffer + front, size);

		front = (front + size - 1);
		if (front >= BUFSIZE)
			front -= BUFSIZE;
	}

	return true;
}

bool RINGBUFFER::Peek(char *dest, int dest_size, int size, int *return_size)
{
	int gap, temp, using_data = GetUseSize();

	if (rear == front || using_data < size)
		return false;

	temp = front + 1;
	if (temp == BUFSIZE)
		temp = 0;

	gap = BUFSIZE - temp;
	if (gap < size)
	{
		memcpy_s(dest, dest_size, buffer + temp, gap);
		size -= gap;	*return_size += gap;

		memcpy_s(dest + gap, dest_size, buffer, size);
		*return_size += size;
	}
	else
	{
		memcpy_s(dest, dest_size, buffer + temp, size);
		*return_size += size;
	}

	return true;
}

void RINGBUFFER::ClearBuffer()
{
	front = rear = 0;
}

char* RINGBUFFER::GetFrontPtr()
{
	return buffer + front;
}

char* RINGBUFFER::GetRearPtr()
{
	return buffer + rear;
}

void RINGBUFFER::MoveFront(int size)
{
	front = (front + size);

	if (front >= BUFSIZE)
		front -= BUFSIZE;
}

void RINGBUFFER::MoveRear(int size)
{
	rear = (rear + size);

	if (rear >= BUFSIZE)
		rear -= BUFSIZE;
}
