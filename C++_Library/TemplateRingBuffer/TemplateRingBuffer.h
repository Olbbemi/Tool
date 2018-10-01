#ifndef TemplateRingBuffer_Info
#define TemplateRingBuffer_Info

#define MAX_SIZE 10000

#include <algorithm>
using namespace std;

template<typename T>
class C_TempRingBuffer
{
private:
	int m_front, m_rear, m_size;
	T *m_array;

	void M_Resize()
	{
		int lo_length, lo_size = m_size;
		m_size += MAX_SIZE;

		T *lo_array = new T[m_size];

		if (m_front <= m_rear)
			lo_length = m_rear - m_front;
		else
			lo_length = lo_size - (m_front - m_rear);

		for (int i = 1; i <= lo_length; i++)
		{
			m_front = (m_front + 1) % lo_size;
			lo_array[i] = m_array[m_front];
		}

		m_front = 0;
		m_rear = lo_length;

		swap(lo_array, m_array);
		delete[] lo_array;
	}

public:
	C_TempRingBuffer()
	{
		m_front = m_rear = 0;
		m_size = MAX_SIZE;

		m_array = new T[m_size];
	}

	~C_TempRingBuffer()
	{
		delete[] m_array;
	}

	void Enqueue(T pa_data)
	{
		if ((m_rear + 1) % m_size == m_front)
			M_Resize();

		m_rear = (m_rear + 1) % m_size;
		m_array[m_rear] = pa_data;
	}

	bool Dequeue(T& pa_data)
	{
		if (m_rear == m_front)
			return false;

		m_front = (m_front + 1) % m_size;
		pa_data = m_array[m_front];
		return true;
	}
};

#endif