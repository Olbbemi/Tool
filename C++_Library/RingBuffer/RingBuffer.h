#ifndef Ring_Buffer_Info
#define Ring_Buffer_Info

#include <Windows.h>

namespace Olbbemi
{
	class C_RINGBUFFER
	{
	private:
		char *m_buffer;
		int m_front, m_rear;
		SRWLOCK m_srw;

	public:
		C_RINGBUFFER();
		~C_RINGBUFFER();

		bool M_Enqueue(char *pa_data, const int pa_size);
		bool M_Dequeue(char *p_dest, const int pa_size);
		bool M_Peek(char *pa_dest, const int pa_size, int &pa_return_size);

		int M_GetUseSize() const;
		int M_GetUnuseSize() const;

		char* M_GetBasicPtr() const;
		char* M_GetFrontPtr() const;
		char* M_GetRearPtr() const;

		void M_MoveFront(const int pa_size);
		void M_MoveRear(const int pa_size);

		int M_LinearRemainFrontSize();
		int M_LinearRemainRearSize();

		void M_RingBuffer_Lock();
		void M_RingBuffer_Unlock();
	};
}

#endif