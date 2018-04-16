#include "Serialize.h"

#include <cstring>
#include <algorithm>
using namespace std;

#define MAXSIZE 20000

SERIALIZE::SERIALIZE() : buffer_size(2000), front(0), rear(0)
{
	buffer_ptr = new char[buffer_size];
}

SERIALIZE::SERIALIZE(int size) : front(0), rear(0)
{
	buffer_ptr = new char[size];
}

int SERIALIZE::GetUsingData()
{
	return rear - front;
}

int SERIALIZE::GetUnusingData()
{
	return buffer_size - rear - 1;
}

void SERIALIZE::Enqueue(char *src, int size)
{
	if (GetUnusingData() < size)
		resize(size);

	memcpy_s(buffer_ptr + rear, buffer_size, src, size);
	rear += size;
}

bool SERIALIZE::Dequeue(char *dest, int size)
{
	if (GetUsingData() < size)
		return false;

	memcpy_s(dest, size, buffer_ptr + front, size);
	front += size;
	return true;
}

void SERIALIZE::MoveFront(int size)
{
	front += size;
}

void SERIALIZE::MoveRear(int size)
{
	rear += size;
}

void SERIALIZE::resize(int size)
{
	int change_size = buffer_size + size + 1000;

	if (change_size > MAXSIZE)
	{
		// exception
	}

	char *temp_ptr = new char[change_size];
	memcpy_s(temp_ptr, change_size, buffer_ptr, buffer_size);

	buffer_size += size + 1000;
	swap(buffer_ptr, temp_ptr);

	delete[] temp_ptr;
}

char* SERIALIZE::GetBufferPtr()
{
	return buffer_ptr;
}