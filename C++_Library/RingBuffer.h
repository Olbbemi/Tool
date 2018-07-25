#ifndef Ring_Buffer_Info
#define Ring_Buffer_Info

#include <Windows.h>

namespace Olbbemi
{
	class RINGBUFFER
	{
	private:
		char *m_buffer;
		int m_front, m_rear;
		SRWLOCK m_srw;

	public:
		RINGBUFFER();
		~RINGBUFFER();

		bool Enqueue(char *p_data, const int p_size);
		bool Dequeue(char *p_dest, const int p_size);
		bool Peek(char *p_dest, const int p_size, int &p_return_size);

		int GetUseSize() const;
		int GetUnuseSize() const;

		char* GetBasicPtr() const;
		char* GetFrontPtr() const;
		char* GetRearPtr() const;

		void MoveFront(const int p_size);
		void MoveRear(const int p_size);

		int LinearRemainFrontSize();
		int LinearRemainRearSize();

		void RingBuffer_Lock();
		void RingBuffer_Unlock();
	};
}

#endif