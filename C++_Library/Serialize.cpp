#include "Precompile.h"
#include "Serialize.h"

#include <cstring>
#include <algorithm>
using namespace std;

SERIALIZE::SERIALIZE(int p_header_size) : m_front(0), m_rear(0)
{
	m_front = m_rear = p_header_size;
}

int SERIALIZE::GetUsingSize()
{
	return m_rear - m_front;
}

int SERIALIZE::GetUnusingSize()
{
	return SERIALIZE_BUFFER_SIZE - m_rear - 1;
}

void SERIALIZE::MakeHeader(const char *p_src, const int p_size)
{
	m_front = 0;
	memcpy_s(m_buffer_ptr, SERIALIZE_BUFFER_SIZE, p_src, p_size);
}

void SERIALIZE::Enqueue(char *p_src, const int p_size)
{
	switch (p_size)
	{
		case 1:		*(reinterpret_cast<char*>(m_buffer_ptr + m_rear)) = *(reinterpret_cast<char*>(p_src));					break;
		case 2:		*(reinterpret_cast<short*>(m_buffer_ptr + m_rear)) = *(reinterpret_cast<short*>(p_src));				break;
		case 4:		*(reinterpret_cast<int*>(m_buffer_ptr + m_rear)) = *(reinterpret_cast<int*>(p_src));					break;
		case 8:		*(reinterpret_cast<long long*>(m_buffer_ptr + m_rear)) = *(reinterpret_cast<long long*>(p_src));		break;
		default:	memcpy_s(m_buffer_ptr + m_rear, SERIALIZE_BUFFER_SIZE, p_src, p_size);									break;
	}

	m_rear += p_size;
}

void SERIALIZE::Dequeue(char *p_dest, const int p_size)
{
	if (GetUsingSize() < p_size)
		return;
	
	switch (p_size)
	{
		case 1:		*(reinterpret_cast<char*>(p_dest)) = *(reinterpret_cast<char*>(m_buffer_ptr + m_front));			break;
		case 2:		*(reinterpret_cast<short*>(p_dest)) = *(reinterpret_cast<short*>(m_buffer_ptr + m_front));			break;
		case 4:		*(reinterpret_cast<int*>(p_dest)) = *(reinterpret_cast<int*>(m_buffer_ptr + m_front));				break;
		case 8:		*(reinterpret_cast<long long*>(p_dest)) = *(reinterpret_cast<long long*>(m_buffer_ptr + m_front));	break;
		default:	memcpy_s(p_dest, p_size, m_buffer_ptr + m_front, p_size);											break;
	}

	m_front += p_size;
}

void SERIALIZE::MoveFront(const int p_size)
{
	m_front += p_size;
}

void SERIALIZE::MoveRear(const int p_size)
{
	m_rear += p_size;
}

char* SERIALIZE::GetBufferPtr()
{
	return m_buffer_ptr + m_front;
}
