#include "Precompile.h"
#include "Log/Log.h"
#include "RingBuffer.h"

#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 10000

using namespace Olbbemi;

C_RINGBUFFER::C_RINGBUFFER()
{
	m_buffer = (char*)malloc(BUFSIZE);
	if (m_buffer == nullptr)
	{
		TCHAR lo_action[] = _TEXT("RINGBUFFER"), lo_server[] = _TEXT("NONE");
		ST_Log lo_log({ "Ringbuffer Malloc Fail" });

		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}

	m_front = m_rear = 0;
}

C_RINGBUFFER::~C_RINGBUFFER()
{
	free(m_buffer);
}

int C_RINGBUFFER::M_GetUseSize() const
{
	int t_front = m_front, t_rear = m_rear;

	if (t_front > t_rear)
		return BUFSIZE - (t_front - t_rear);	// (BUFSIZE - 1) - (front - rear) + 1; 
	else
		return t_rear - t_front;
}

int C_RINGBUFFER::M_GetUnuseSize() const
{
	int t_front = m_front, t_rear = m_rear;

	if (t_front > t_rear)
		return t_front - t_rear - 1;
	else
		return (BUFSIZE - 1) - (t_rear - t_front);
}

bool C_RINGBUFFER::M_Enqueue(char *pa_data, const int pa_size)
{
	int gap, unusing_data = M_GetUnuseSize(), t_front = m_front, t_rear = m_rear;

	if ((t_rear + 1) % BUFSIZE == m_front || unusing_data < pa_size)
		return false;

	t_rear++;
	if (t_rear == BUFSIZE)
		t_rear = 0;

	gap = BUFSIZE - t_rear;
	if (gap < pa_size)
	{
		switch (gap)
		{
			case 1:		*(reinterpret_cast<char*>(m_buffer + t_rear)) = *(reinterpret_cast<char*>(pa_data));				break;
			case 2:		*(reinterpret_cast<short*>(m_buffer + t_rear)) = *(reinterpret_cast<short*>(pa_data));				break;
			case 4:		*(reinterpret_cast<int*>(m_buffer + t_rear)) = *(reinterpret_cast<int*>(pa_data));					break;
			case 8:		*(reinterpret_cast<long long*> (m_buffer + t_rear)) = *(reinterpret_cast<long long*>(pa_data));		break;
			default:	memcpy_s(m_buffer + t_rear, BUFSIZE, pa_data, gap);													break;
		}

		switch (pa_size - gap)
		{
			case 1:		*(reinterpret_cast<char*>(m_buffer)) = *(reinterpret_cast<char*>(pa_data + gap));				break;
			case 2:		*(reinterpret_cast<short*>(m_buffer)) = *(reinterpret_cast<short*>(pa_data + gap));				break;
			case 4:		*(reinterpret_cast<int*>(m_buffer)) = *(reinterpret_cast<int*>(pa_data + gap));					break;
			case 8:		*(reinterpret_cast<long long*>(m_buffer)) = *(reinterpret_cast<long long*>(pa_data + gap));		break;
			default:	memcpy_s(m_buffer, BUFSIZE, pa_data + gap, pa_size - gap);										break;
		}

		m_rear = pa_size - gap - 1;
	}
	else
	{
		switch (pa_size)
		{
			case 1:		*(reinterpret_cast<char*>(m_buffer + t_rear)) = *(reinterpret_cast<char*>(pa_data));				break;
			case 2:		*(reinterpret_cast<short*>(m_buffer + t_rear)) = *(reinterpret_cast<short*>(pa_data));				break;
			case 4:		*(reinterpret_cast<int*>(m_buffer + t_rear)) = *(reinterpret_cast<int*>(pa_data));					break;
			case 8:		*(reinterpret_cast<long long*>(m_buffer + t_rear)) = *(reinterpret_cast<long long*> (pa_data));		break;
			default:	memcpy_s(m_buffer + t_rear, BUFSIZE, pa_data, pa_size);												break;
		}

		if (BUFSIZE <= m_rear + pa_size)
			m_rear += pa_size - BUFSIZE;
		else
			m_rear += pa_size;
	}

	return true;
}

bool C_RINGBUFFER::M_Dequeue(char *pa_dest, const int pa_size)
{
	int gap, using_data = M_GetUseSize(), t_front = m_front, t_rear = m_rear;

	if (t_rear == t_front || using_data < pa_size)
		return false;

	t_front++;
	if (t_front == BUFSIZE)
		t_front = 0;

	gap = BUFSIZE - t_front;
	if (gap < pa_size)
	{
		switch (gap)
		{
			case 1:		*(reinterpret_cast<char*>(pa_dest)) = *(reinterpret_cast<char*>(m_buffer + t_front));				break;
			case 2:		*(reinterpret_cast<short*>(pa_dest)) = *(reinterpret_cast<short*>(m_buffer + t_front));				break;
			case 4:		*(reinterpret_cast<int*>(pa_dest)) = *(reinterpret_cast<int*>(m_buffer + t_front));					break;
			case 8:		*(reinterpret_cast<long long*> (pa_dest)) = *(reinterpret_cast<long long*>(m_buffer + t_front));	break;
			default:	memcpy_s(pa_dest, gap, m_buffer + t_front, gap);													break;
		}

		switch (pa_size - gap)
		{
			case 1:		*(reinterpret_cast<char*>(pa_dest + gap)) = *(reinterpret_cast<char*>(m_buffer));				break;
			case 2:		*(reinterpret_cast<short*>(pa_dest + gap)) = *(reinterpret_cast<short*>(m_buffer));				break;
			case 4:		*(reinterpret_cast<int*>(pa_dest + gap)) = *(reinterpret_cast<int*>(m_buffer));					break;
			case 8:		*(reinterpret_cast<long long*> (pa_dest + gap)) = *(reinterpret_cast<long long*>(m_buffer));	break;
			default:	memcpy_s(pa_dest + gap, pa_size - gap, m_buffer, pa_size - gap);								break;
		}

		m_front = pa_size - gap - 1;
	}
	else
	{
		switch (pa_size)
		{
			case 1:		*(reinterpret_cast<char*>(pa_dest)) = *(reinterpret_cast<char*>(m_buffer + t_front));				break;
			case 2:		*(reinterpret_cast<short*>(pa_dest)) = *(reinterpret_cast<short*>(m_buffer + t_front));				break;
			case 4:		*(reinterpret_cast<int*>(pa_dest)) = *(reinterpret_cast<int*>(m_buffer + t_front));					break;
			case 8:		*(reinterpret_cast<long long*> (pa_dest)) = *(reinterpret_cast<long long*>(m_buffer + t_front));	break;
			default:	memcpy_s(pa_dest, pa_size, m_buffer + t_front, pa_size);											break;
		}

		if (BUFSIZE <= m_front + pa_size)
			m_front += pa_size - BUFSIZE;
		else
			m_front += pa_size;
	}

	return true;
}

bool C_RINGBUFFER::M_Peek(char *pa_dest, const int pa_size, int &pa_return_size)
{
	int gap, temp, using_data = M_GetUseSize(), t_front = m_front, t_rear = m_rear;

	if (t_rear == t_front || using_data < pa_size)
		return false;

	temp = t_front + 1;
	if (temp == BUFSIZE)
		temp = 0;

	gap = BUFSIZE - temp;
	if (gap < pa_size)
	{
		switch (gap)
		{
			case 1:		*(reinterpret_cast<char*>(pa_dest)) = *(reinterpret_cast<char*>(m_buffer + temp));				break;
			case 2:		*(reinterpret_cast<short*>(pa_dest)) = *(reinterpret_cast<short*>(m_buffer + temp));			break;
			case 4:		*(reinterpret_cast<int*>(pa_dest)) = *(reinterpret_cast<int*>(m_buffer + temp));				break;
			case 8:		*(reinterpret_cast<long long*>(pa_dest)) = *(reinterpret_cast<long long*>(m_buffer + temp));	break;
			default:	memcpy_s(pa_dest, gap, m_buffer + temp, gap);													break;
		}

		switch (pa_size - gap)
		{
			case 1:		*(reinterpret_cast<char*>(pa_dest + gap)) = *(reinterpret_cast<char*>(m_buffer));				break;
			case 2:		*(reinterpret_cast<short*>(pa_dest + gap)) = *(reinterpret_cast<short*>(m_buffer));				break;
			case 4:		*(reinterpret_cast<int*>(pa_dest + gap)) = *(reinterpret_cast<int*>(m_buffer));					break;
			case 8:		*(reinterpret_cast<long long*>(pa_dest + gap)) = *(reinterpret_cast<long long*>(m_buffer));		break;
			default:	memcpy_s(pa_dest + gap, pa_size - gap, m_buffer, pa_size - gap);								break;
		}
	}
	else
	{
		switch (pa_size)
		{
			case 1:		*(reinterpret_cast<char*>(pa_dest)) = *(reinterpret_cast<char*>(m_buffer + temp));				break;
			case 2:		*(reinterpret_cast<short*>(pa_dest)) = *(reinterpret_cast<short*>(m_buffer + temp));			break;
			case 4:		*(reinterpret_cast<int*>(pa_dest)) = *(reinterpret_cast<int*>(m_buffer + temp));				break;
			case 8:		*(reinterpret_cast<long long*>(pa_dest)) = *(reinterpret_cast<long long*>(m_buffer + temp));	break;
			default:	memcpy_s(pa_dest, pa_size, m_buffer + temp, pa_size);											break;
		}
	}

	pa_return_size += pa_size;
	return true;
}

char* C_RINGBUFFER::M_GetBasicPtr() const
{
	return m_buffer;
}

char* C_RINGBUFFER::M_GetFrontPtr() const
{
	int front_temp = m_front + 1;
	if (front_temp == BUFSIZE)
		front_temp = 0;

	return m_buffer + front_temp;
}

char* C_RINGBUFFER::M_GetRearPtr() const
{
	int rear_temp = m_rear + 1;
	if (rear_temp == BUFSIZE)
		rear_temp = 0;

	return m_buffer + rear_temp;
}

void C_RINGBUFFER::M_MoveFront(const int pa_size)
{
	int t_front = m_front;
	t_front += pa_size;
	if (t_front >= BUFSIZE)
		t_front -= BUFSIZE;

	m_front = t_front;
}

void C_RINGBUFFER::M_MoveRear(const int pa_size)
{
	int t_rear = m_rear;
	t_rear += pa_size;
	if (t_rear >= BUFSIZE)
		t_rear -= BUFSIZE;

	m_rear = t_rear;
}

int C_RINGBUFFER::M_LinearRemainFrontSize()
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

int C_RINGBUFFER::M_LinearRemainRearSize()
{
	int t_front = m_front, t_rear = m_rear;

	int rear_temp = t_rear + 1;
	if (rear_temp == BUFSIZE)	rear_temp = 0;

	if (rear_temp >= t_front)
		return BUFSIZE - rear_temp;
	else
		return t_front - rear_temp;
}

void C_RINGBUFFER::M_RingBuffer_Lock()
{
	AcquireSRWLockExclusive(&m_srw);
}

void C_RINGBUFFER::M_RingBuffer_Unlock()
{
	ReleaseSRWLockExclusive(&m_srw);
}