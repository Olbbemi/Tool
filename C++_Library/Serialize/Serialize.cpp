#include "Precompile.h"
#include "Log/Log.h"
#include "Serialize.h"
#include "Protocol/Define.h"

#include <stdlib.h>
#include <string.h>

using namespace Olbbemi;

C_MemoryPoolTLS<C_Serialize> C_Serialize::s_memory_pool(false);

C_Serialize* C_Serialize::S_Alloc()
{
	C_Serialize* lo_serialQ = s_memory_pool.M_Alloc();
	lo_serialQ->m_ref_count = 1;

	return lo_serialQ;
}

void C_Serialize::S_Free(C_Serialize* pa_serialQ)
{
	LONG lo_ref_count = InterlockedDecrement(&pa_serialQ->m_ref_count);
	if (lo_ref_count == 0)
	{
		pa_serialQ->m_encode_enable = true;
		pa_serialQ->m_send_and_disconnect = false;

		pa_serialQ->m_front = pa_serialQ->m_rear = MAX_HEAD_SIZE;
		
		s_memory_pool.M_Free(pa_serialQ);
	}
}

void C_Serialize::S_Terminate()
{
	s_memory_pool.M_Terminate();
}

void C_Serialize::S_AddReference(C_Serialize* pa_serialQ)
{
	InterlockedIncrement(&pa_serialQ->m_ref_count);
}

LONG C_Serialize::S_TLSAllocCount()
{
	return s_memory_pool.M_AllocCount();
}

LONG C_Serialize::S_TLSChunkCount()
{
	return s_memory_pool.M_UseChunkCount();
}

LONG C_Serialize::S_TLSNodeCount()
{
	return s_memory_pool.M_UseNodeCount();
}

C_Serialize::C_Serialize()
{
	m_encode_enable = true;
	m_send_and_disconnect = false;

	m_front = m_rear = MAX_HEAD_SIZE;
	m_maximum_size = SERIALIZE_BUFFER_SIZE;

	m_buffer_ptr = (char*)malloc(SERIALIZE_BUFFER_SIZE);
	if (m_buffer_ptr == nullptr)
	{
		TCHAR lo_action[] = _TEXT("SERIALIZE"), lo_server[] = _TEXT("Common");
		ST_Log lo_log({"Seiralize Malloc Fail"});

		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}
}

C_Serialize::~C_Serialize()
{
	free(m_buffer_ptr);
}

void C_Serialize::M_Resize(int p_remain_size, int p_input_size)
{
	while (p_remain_size + m_maximum_size <= p_input_size)
		m_maximum_size += SERIALIZE_BUFFER_SIZE;

	char* temp_buffer = (char*)malloc(m_maximum_size);
	if (temp_buffer == nullptr)
	{
		TCHAR lo_action[] = _TEXT("SERIALIZE"), lo_server[] = _TEXT("Common");
		ST_Log lo_log({ "Seiralize Resize malloc Fail" });
		
		_LOG(__LINE__, LOG_LEVEL_SYSTEM, lo_action, lo_server, lo_log.count, lo_log.log_str);
	}

	memcpy_s(temp_buffer, m_maximum_size, m_buffer_ptr, m_maximum_size);

	free(m_buffer_ptr);
	m_buffer_ptr = temp_buffer;
}

int C_Serialize::M_GetUsingSize() const
{
	return m_rear - m_front;
}

int C_Serialize::M_GetUnusingSize() const
{
	return SERIALIZE_BUFFER_SIZE - m_rear - 1;
}

void C_Serialize::M_SendAndDisconnect()
{
	m_send_and_disconnect = true;
}

void C_Serialize::M_NetMakeHeader(const char *p_src, const int p_size)
{
	m_front = MAX_HEAD_SIZE - NET_HEAD_SIZE;
	memcpy_s(m_buffer_ptr + m_front, SERIALIZE_BUFFER_SIZE, p_src, p_size);
}

void C_Serialize::M_LanMakeHeader(const char *p_src, const int p_size)
{
	m_front = MAX_HEAD_SIZE - LAN_HEAD_SIZE;
	memcpy_s(m_buffer_ptr + m_front, SERIALIZE_BUFFER_SIZE, p_src, p_size);
}

void C_Serialize::M_Enqueue(char *p_src, const int p_size)
{
	int remain_size = M_GetUnusingSize();

	if (remain_size < p_size)
		M_Resize(remain_size, p_size);

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

void C_Serialize::M_Dequeue(char *p_dest, const int p_size)
{
	if (M_GetUsingSize() < p_size)
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

void C_Serialize::M_MoveFront(const int p_size)
{
	m_front += p_size;
}

void C_Serialize::M_MoveRear(const int p_size)
{
	m_rear += p_size;
}

char* C_Serialize::M_GetBufferPtr() const
{
	return m_buffer_ptr + m_front;
}