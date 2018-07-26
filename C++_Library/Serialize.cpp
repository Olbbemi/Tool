#include "Precompile.h"
#include "Log.h"
#include "Serialize.h"

#include <stdlib.h>
#include <string.h>
using namespace Olbbemi;

SERIALIZE::SERIALIZE()
{
	m_front = m_rear = 0;
	m_total_length = SERIALIZE_BUFFER_SIZE;

	m_buffer_ptr = (char*)malloc(SERIALIZE_BUFFER_SIZE);
	if (m_buffer_ptr == nullptr)
	{
		TCHAR action[] = _TEXT("SERIALIZE"), server[] = _TEXT("NONE");
		initializer_list<string> str = { "Seiralize malloc Fail" };

		_LOG(__LINE__, LOG_LEVEL_ERROR, action, server, str);
		throw;
	}
}

SERIALIZE::~SERIALIZE()
{
	free(m_buffer_ptr);
}

void SERIALIZE::Resize(int p_remain_size, int p_input_size)
{
	while (p_remain_size + m_total_length <= p_input_size)
		m_total_length += SERIALIZE_BUFFER_SIZE;

	char* temp_buffer = (char*)malloc(m_total_length);
	if (temp_buffer == nullptr)
	{
		TCHAR action[] = _TEXT("SERIALIZE"), server[] = _TEXT("NONE");
		initializer_list<string> str = { "Seiralize Resize malloc Fail" };

		_LOG(__LINE__, LOG_LEVEL_ERROR, action, server, str);
		throw;
	}

	memcpy_s(temp_buffer, m_total_length, m_buffer_ptr, m_total_length);

	free(m_buffer_ptr);
	m_buffer_ptr = temp_buffer;
}

int SERIALIZE::GetUsingSize() const
{
	return m_rear - m_front;
}

int SERIALIZE::GetUnusingSize() const
{
	return SERIALIZE_BUFFER_SIZE - m_rear - 1;
}

void SERIALIZE::InputHeaderSize(int p_header_size)
{
	m_front = m_rear = p_header_size;
	m_total_length = SERIALIZE_BUFFER_SIZE;
}

void SERIALIZE::MakeHeader(const char *p_src, const int p_size)
{
	m_front = 0;
	memcpy_s(m_buffer_ptr, SERIALIZE_BUFFER_SIZE, p_src, p_size);
}

void SERIALIZE::Enqueue(char *p_src, const int p_size)
{
	int remain_size = GetUnusingSize();

	if (remain_size < p_size)
		Resize(remain_size, p_size);

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

char* SERIALIZE::GetBufferPtr() const
{
	return m_buffer_ptr + m_front;
}