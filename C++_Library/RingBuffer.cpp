#include "Precompile.h"
#include "RingBuffer.h"

#include <stdlib.h>

#define BUFSIZE 10000

using namespace Olbbemi;

RINGBUFFER::RINGBUFFER()
{
	m_buffer = (char*)malloc(BUFSIZE);
	m_front = m_rear = 0;
}

RINGBUFFER::~RINGBUFFER()
{
	free(m_buffer);
}

int RINGBUFFER::GetUseSize() const
{
	int t_front = m_front, t_rear = m_rear;

	if (t_front > t_rear)
		return BUFSIZE - (t_front - t_rear);	// (BUFSIZE - 1) - (front - rear) + 1; 
	else
		return t_rear - t_front;
}

int RINGBUFFER::GetUnuseSize() const
{
	int t_front = m_front, t_rear = m_rear;

	if (t_front > t_rear)
		return t_front - t_rear - 1;
	else
		return (BUFSIZE - 1) - (t_rear - t_front);
}

bool RINGBUFFER::Enqueue(char *p_data, const int p_size)
{
	int gap, unusing_data = GetUnuseSize(), t_front = m_front, t_rear = m_rear;

	if ((t_rear + 1) % BUFSIZE == m_front || unusing_data < p_size)
		return false;

	t_rear++;
	if (t_rear == BUFSIZE)
		t_rear = 0;

	gap = BUFSIZE - t_rear;
	if (gap < p_size)
	{
		switch (gap)
		{
			case 1:		*(reinterpret_cast<char*>(m_buffer + t_rear)) = *(reinterpret_cast<char*>(p_data));				break;
			case 2:		*(reinterpret_cast<short*>(m_buffer + t_rear)) = *(reinterpret_cast<short*>(p_data));			break;
			case 4:		*(reinterpret_cast<int*>(m_buffer + t_rear)) = *(reinterpret_cast<int*>(p_data));				break;
			case 8:		*(reinterpret_cast<long long*> (m_buffer + t_rear)) = *(reinterpret_cast<long long*>(p_data));	break;
			default:	memcpy_s(m_buffer + t_rear, BUFSIZE, p_data, gap);												break;
		}

		switch (p_size - gap)
		{
			case 1:		*(reinterpret_cast<char*>(m_buffer)) = *(reinterpret_cast<char*>(p_data + gap));				break;
			case 2:		*(reinterpret_cast<short*>(m_buffer)) = *(reinterpret_cast<short*>(p_data + gap));			break;
			case 4:		*(reinterpret_cast<int*>(m_buffer)) = *(reinterpret_cast<int*>(p_data + gap));				break;
			case 8:		*(reinterpret_cast<long long*>(m_buffer)) = *(reinterpret_cast<long long*>(p_data + gap));	break;
			default:	memcpy_s(m_buffer, BUFSIZE, p_data + gap, p_size - gap);											break;
		}

		m_rear = p_size - gap - 1;
	}
	else
	{
		switch (p_size)
		{
			case 1:		*(reinterpret_cast<char*>(m_buffer + t_rear)) = *(reinterpret_cast<char*>(p_data));				break;
			case 2:		*(reinterpret_cast<short*>(m_buffer + t_rear)) = *(reinterpret_cast<short*>(p_data));			break;
			case 4:		*(reinterpret_cast<int*>(m_buffer + t_rear)) = *(reinterpret_cast<int*>(p_data));				break;
			case 8:		*(reinterpret_cast<long long*>(m_buffer + t_rear)) = *(reinterpret_cast<long long*> (p_data));	break;
			default:	memcpy_s(m_buffer + t_rear, BUFSIZE, p_data, p_size);											break;
		}

		if (BUFSIZE <= m_rear + p_size)
			m_rear += p_size - BUFSIZE;
		else
			m_rear += p_size;
	}

	return true;
}

bool RINGBUFFER::Dequeue(char *p_dest, const int p_size)
{
	int gap, using_data = GetUseSize(), t_front = m_front, t_rear = m_rear;

	if (t_rear == t_front || using_data < p_size)
		return false;

	t_front++;
	if (t_front == BUFSIZE)
		t_front = 0;

	gap = BUFSIZE - t_front;
	if (gap < p_size)
	{
		switch (gap)
		{
			case 1:		*(reinterpret_cast<char*>(p_dest)) = *(reinterpret_cast<char*>(m_buffer + t_front));			break;
			case 2:		*(reinterpret_cast<short*>(p_dest)) = *(reinterpret_cast<short*>(m_buffer + t_front));			break;
			case 4:		*(reinterpret_cast<int*>(p_dest)) = *(reinterpret_cast<int*>(m_buffer + t_front));				break;
			case 8:		*(reinterpret_cast<long long*> (p_dest)) = *(reinterpret_cast<long long*>(m_buffer + t_front));	break;
			default:	memcpy_s(p_dest, gap, m_buffer + t_front, gap);													break;
		}

		switch (p_size - gap)
		{
			case 1:		*(reinterpret_cast<char*>(p_dest + gap)) = *(reinterpret_cast<char*>(m_buffer));			break;
			case 2:		*(reinterpret_cast<short*>(p_dest + gap)) = *(reinterpret_cast<short*>(m_buffer));			break;
			case 4:		*(reinterpret_cast<int*>(p_dest + gap)) = *(reinterpret_cast<int*>(m_buffer));				break;
			case 8:		*(reinterpret_cast<long long*> (p_dest + gap)) = *(reinterpret_cast<long long*>(m_buffer));	break;
			default:	memcpy_s(p_dest + gap, p_size - gap, m_buffer, p_size - gap);									break;
		}

		m_front = p_size - gap - 1;
	}
	else
	{
		switch (p_size)
		{
			case 1:		*(reinterpret_cast<char*>(p_dest)) = *(reinterpret_cast<char*>(m_buffer + t_front));			break;
			case 2:		*(reinterpret_cast<short*>(p_dest)) = *(reinterpret_cast<short*>(m_buffer + t_front));			break;
			case 4:		*(reinterpret_cast<int*>(p_dest)) = *(reinterpret_cast<int*>(m_buffer + t_front));				break;
			case 8:		*(reinterpret_cast<long long*> (p_dest)) = *(reinterpret_cast<long long*>(m_buffer + t_front));	break;
			default:	memcpy_s(p_dest, p_size, m_buffer + t_front, p_size);											break;
		}

		if (BUFSIZE <= m_front + p_size)
			m_front += p_size - BUFSIZE;
		else
			m_front += p_size;
	}

	return true;
}

bool RINGBUFFER::Peek(char *p_dest, const int p_size, int &p_return_size)
{
	int gap, temp, using_data = GetUseSize(), t_front = m_front, t_rear = m_rear;

	if (t_rear == t_front || using_data < p_size)
		return false;

	temp = t_front + 1;
	if (temp == BUFSIZE)
		temp = 0;

	gap = BUFSIZE - temp;
	if (gap < p_size)
	{
		switch (gap)
		{
			case 1:		*(reinterpret_cast<char*>(p_dest)) = *(reinterpret_cast<char*>(m_buffer + temp));			break;
			case 2:		*(reinterpret_cast<short*>(p_dest)) = *(reinterpret_cast<short*>(m_buffer + temp));			break;
			case 4:		*(reinterpret_cast<int*>(p_dest)) = *(reinterpret_cast<int*>(m_buffer + temp));				break;
			case 8:		*(reinterpret_cast<long long*>(p_dest)) = *(reinterpret_cast<long long*>(m_buffer + temp));	break;
			default:	memcpy_s(p_dest, gap, m_buffer + temp, gap);												break;
		}

		switch (p_size - gap)
		{
			case 1:		*(reinterpret_cast<char*>(p_dest + gap)) = *(reinterpret_cast<char*>(m_buffer));			break;
			case 2:		*(reinterpret_cast<short*>(p_dest + gap)) = *(reinterpret_cast<short*>(m_buffer));			break;
			case 4:		*(reinterpret_cast<int*>(p_dest + gap)) = *(reinterpret_cast<int*>(m_buffer));				break;
			case 8:		*(reinterpret_cast<long long*>(p_dest + gap)) = *(reinterpret_cast<long long*>(m_buffer));	break;
			default:	memcpy_s(p_dest + gap, p_size - gap, m_buffer, p_size - gap);								break;
		}
	}
	else
	{
		switch (p_size)
		{
			case 1:		*(reinterpret_cast<char*>(p_dest)) = *(reinterpret_cast<char*>(m_buffer + temp));			break;
			case 2:		*(reinterpret_cast<short*>(p_dest)) = *(reinterpret_cast<short*>(m_buffer + temp));			break;
			case 4:		*(reinterpret_cast<int*>(p_dest)) = *(reinterpret_cast<int*>(m_buffer + temp));				break;
			case 8:		*(reinterpret_cast<long long*>(p_dest)) = *(reinterpret_cast<long long*>(m_buffer + temp));	break;
			default:	memcpy_s(p_dest, p_size, m_buffer + temp, p_size);											break;
		}
	}

	p_return_size += p_size;
	return true;
}

char* RINGBUFFER::GetBasicPtr() const
{
	return m_buffer;
}

char* RINGBUFFER::GetFrontPtr() const
{
	int front_temp = m_front + 1;
	if (front_temp == BUFSIZE)
		front_temp = 0;

	return m_buffer + front_temp;
}

char* RINGBUFFER::GetRearPtr() const
{
	int rear_temp = m_rear + 1;
	if (rear_temp == BUFSIZE)
		rear_temp = 0;

	return m_buffer + rear_temp;
}

void RINGBUFFER::MoveFront(const int p_size)
{
	int t_front = m_front;
	t_front += p_size;
	if (t_front >= BUFSIZE)
		t_front -= BUFSIZE;

	m_front = t_front;
}

void RINGBUFFER::MoveRear(const int p_size)
{
	int t_rear = m_rear;
	t_rear += p_size;
	if (t_rear >= BUFSIZE)
		t_rear -= BUFSIZE;

	m_rear = t_rear;
}

int RINGBUFFER::LinearRemainFrontSize()
{
	int t_front = m_front, t_rear = m_rear;

	if (t_front == t_rear)
		return 0;
	else
	{
		int front_temp = t_front + 1;
		if (front_temp == BUFSIZE)	front_temp = 0;

		if (front_temp > t_rear)
			return BUFSIZE - front_temp;
		else
			return t_rear - front_temp + 1;
	}
}

int RINGBUFFER::LinearRemainRearSize()
{
	int t_front = m_front, t_rear = m_rear;

	int rear_temp = t_rear + 1;
	if (rear_temp == BUFSIZE)	rear_temp = 0;

	if (rear_temp >= t_front)
		return BUFSIZE - rear_temp;
	else
		return t_front - rear_temp;
}

void RINGBUFFER::RingBuffer_Lock()
{
	AcquireSRWLockExclusive(&m_srw);
}

void RINGBUFFER::RingBuffer_Unlock()
{
	ReleaseSRWLockExclusive(&m_srw);
}
