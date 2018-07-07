using System;

namespace RINGBUFFER
{
    class RingBuffer
    {
        Byte[] m_buffer;
        Int32 m_front, m_rear;

        private enum Initial_Value : Int32
        {
            en_buffer_size = 100
        }

        public RingBuffer()
        {
            m_buffer = new byte[(Int32)Initial_Value.en_buffer_size];
            m_front = m_rear = 0;
        }


        /*
            버퍼에 데이터를 삽입 및 삭제하는 함수
        */
        unsafe public bool Enqueue(IntPtr p_input_data, Int32 p_size)
        {
            Int32 gap, t_front = m_front, t_rear = m_rear, unusing_data = GetUnuseSize();

            if ((t_rear + 1) % (Int32)Initial_Value.en_buffer_size == t_front || unusing_data < p_size)
                return false;

            t_rear++;
            if (t_rear == (Int32)Initial_Value.en_buffer_size)
                t_rear = 0;
            gap = (Int32)Initial_Value.en_buffer_size - t_rear;
            fixed (Byte* fix_buffer = m_buffer)
            {
                if (gap < p_size)
                {
                    switch (gap)
                    {
                        case 1: *(Byte*)(fix_buffer + t_rear)  = *(Byte*)p_input_data; break;
                        case 2: *(Int16*)(fix_buffer + t_rear) = *(Int16*)p_input_data; break;
                        case 4: *(Int32*)(fix_buffer + t_rear) = *(Int32*)p_input_data; break;
                        case 8: *(Int64*)(fix_buffer + t_rear) = *(Int64*)p_input_data; break;
                        default: System.Runtime.InteropServices.Marshal.Copy(p_input_data, m_buffer, t_rear, gap); break;
                    }

                    Byte* byte_store = (Byte*)(p_input_data.ToPointer());
                    byte_store += gap;

                    IntPtr intptr_store = (IntPtr)byte_store;

                    System.Runtime.InteropServices.Marshal.Copy(intptr_store, m_buffer, 0, p_size - gap);
                    m_rear = p_size - gap - 1;
                }
                else
                {
                    switch (p_size)
                    {
                        case 1: *(Byte*)(fix_buffer + t_rear)  = *(Byte*)p_input_data; break;
                        case 2: *(Int16*)(fix_buffer + t_rear) = *(Int16*)p_input_data; break;
                        case 4: *(Int32*)(fix_buffer + t_rear) = *(Int32*)p_input_data; break;
                        case 8: *(Int64*)(fix_buffer + t_rear) = *(Int64*)p_input_data; break;
                        default: System.Runtime.InteropServices.Marshal.Copy(p_input_data, m_buffer, t_rear, p_size); break;
                    }

                    if ((Int32)Initial_Value.en_buffer_size <= m_rear + p_size)
                        m_rear += p_size - (Int32)Initial_Value.en_buffer_size;
                    else
                        m_rear += p_size;
                }
            }

            return true;
        }

        unsafe public bool Dequeue(IntPtr p_output_data, Int32 p_size)
        {
            Byte* byte_store;
            Int32 gap, t_front = m_front, t_rear = m_rear, using_data = GetUseSize();

            if (t_rear == t_front || using_data < p_size)
                return false;

            t_front++;
            if (t_front == (Int32)Initial_Value.en_buffer_size)
                t_front = 0;

            gap = (Int32)Initial_Value.en_buffer_size - t_front;
            fixed (Byte* fix_buffer = m_buffer)
            {
                if (gap < p_size)
                {
                    switch (gap)
                    {
                        case 1: *(Byte*)p_output_data  = *(Byte*)(fix_buffer + t_front); break;
                        case 2: *(Int16*)p_output_data = *(Int16*)(fix_buffer + t_front); break;
                        case 4: *(Int32*)p_output_data = *(Int32*)(fix_buffer + t_front); break;
                        case 8: *(Int64*)p_output_data = *(Int64*)(fix_buffer + t_front); break;
                        default: System.Runtime.InteropServices.Marshal.Copy(m_buffer, t_front, p_output_data, gap); break;
                    }

                    byte_store = (Byte*)(p_output_data.ToPointer()); byte_store += gap;
                    IntPtr intptr_store = (IntPtr)byte_store;

                    System.Runtime.InteropServices.Marshal.Copy(m_buffer, 0, intptr_store, p_size - gap);
                    m_front = p_size - gap - 1;
                }
                else
                {
                    switch (p_size)
                    {
                        case 1: *(Byte*)p_output_data  = *(Byte*)(fix_buffer + t_front); break;
                        case 2: *(Int16*)p_output_data = *(Int16*)(fix_buffer + t_front); break;
                        case 4: *(Int32*)p_output_data = *(Int32*)(fix_buffer + t_front); break;
                        case 8: *(Int64*)p_output_data = *(Int64*)(fix_buffer + t_front); break;
                        default: System.Runtime.InteropServices.Marshal.Copy(m_buffer, t_front, p_output_data, p_size); break;
                    }

                    if ((Int32)Initial_Value.en_buffer_size <= m_front + p_size)
                        m_front += p_size - (Int32)Initial_Value.en_buffer_size;
                    else
                        m_front += p_size;
                }
            }

            return true;
        }

        unsafe public bool Peek(IntPtr p_output_data, Int32 p_size, ref Int32 p_return_size)
        {
            Byte* byte_store;
            Int32 gap, temp, t_front = m_front, t_rear = m_rear, using_data = GetUseSize();

            if (t_rear == t_front || using_data < p_size)
                return false;

            temp = t_front + 1;
            if (temp == (Int32)Initial_Value.en_buffer_size)
                temp = 0;

            gap = (Int32)Initial_Value.en_buffer_size - temp;
            fixed (Byte* fix_buffer = m_buffer)
            {
                if (gap < p_size)
                {
                    switch (gap)
                    {
                        case 1: *(Byte*)p_output_data  = *(Byte*)(fix_buffer + temp); break;
                        case 2: *(Int16*)p_output_data = *(Int16*)(fix_buffer + temp); break;
                        case 4: *(Int32*)p_output_data = *(Int32*)(fix_buffer + temp); break;
                        case 8: *(Int64*)p_output_data = *(Int64*)(fix_buffer + temp); break;
                        default: System.Runtime.InteropServices.Marshal.Copy(m_buffer, temp, p_output_data, gap); break;
                    }

                    byte_store = (Byte*)(p_output_data.ToPointer()); byte_store += gap;
                    IntPtr intptr_store = (IntPtr)byte_store;

                    System.Runtime.InteropServices.Marshal.Copy(m_buffer, 0, intptr_store, p_size - gap);
                }
                else
                {
                    switch (p_size)
                    {
                        case 1: *(Byte*)p_output_data  = *(Byte*)(fix_buffer + temp); break;
                        case 2: *(Int16*)p_output_data = *(Int16*)(fix_buffer + temp); break;
                        case 4: *(Int32*)p_output_data = *(Int32*)(fix_buffer + temp); break;
                        case 8: *(Int64*)p_output_data = *(Int64*)(fix_buffer + temp); break;
                        default: System.Runtime.InteropServices.Marshal.Copy(m_buffer, temp, p_output_data, p_size); break;
                    }
                }

                p_return_size += p_size;
            }

            return true;
        }


        /*
            Enqueue 및 Dequeue 함수 이외 외적으로 버퍼의 front 및 rear 정보가 변경되어야할 경우 호출되는 함수
        */
        public void MoveFront(Int32 p_size)
        {
            Int32 t_front = m_front;

            t_front += p_size;
            if (t_front >= (Int32)Initial_Value.en_buffer_size)
                t_front -= (Int32)Initial_Value.en_buffer_size;

            m_front = t_front;
        }

        public void MoveRear(Int32 p_size)
        {
            Int32 t_rear = m_rear;

            m_rear += p_size;
            if (m_rear >= (Int32)Initial_Value.en_buffer_size)
                m_rear -= (Int32)Initial_Value.en_buffer_size;

            m_rear = t_rear;
        }


        /*
            원형 큐의 전체 사용 및 미사용 크기 얻는 함수 
        */
        public Int32 GetUseSize()
        {
            Int32 t_front = m_front, t_rear = m_rear;

            if (t_front > t_rear)
                return (Int32)Initial_Value.en_buffer_size - (t_front - t_rear);    // (BUFSIZE - 1) - (front - rear) + 1; 
            else
                return t_rear - t_front;
        }

        public Int32 GetUnuseSize()
        {
            Int32 t_front = m_front, t_rear = m_rear;

            if (t_front > m_rear)
                return t_front - t_rear - 1;
            else
                return ((Int32)Initial_Value.en_buffer_size - 1) - (t_rear - t_front);
        }


        /*
            원형 큐의 각 위치에서 끊어지지 않은 영역 (선형 영역) 의 크기를 얻는 함수  
        */
        public Int32 LinearRemainFrontSize()
        {
            Int32 t_front = m_front, t_rear = m_rear;

            if (t_front == t_rear)
                return 0;
            else
            {
                Int32 front_temp = t_front + 1;
                if (front_temp == (Int32)Initial_Value.en_buffer_size) front_temp = 0;

                if (front_temp > t_rear)
                    return (Int32)Initial_Value.en_buffer_size - front_temp;
                else
                    return t_rear - front_temp + 1;
            }
        }

        public Int32 LinearRemainRearSize()
        {
            Int32 t_front = m_front, t_rear = m_rear;

            Int32 rear_temp = t_rear + 1;
            if (rear_temp == (Int32)Initial_Value.en_buffer_size) rear_temp = 0;

            if (rear_temp >= t_front)
                return (Int32)Initial_Value.en_buffer_size - rear_temp;
            else
                return t_front - rear_temp;
        }


        /*
            원형 큐의 각 위치에 대한 버퍼 주소를 얻는 함수       
        */
        unsafe public IntPtr GetBasicPtr()
        {
            fixed (Byte* fix_buffer = m_buffer)
                return (IntPtr)fix_buffer;
        }

        unsafe public IntPtr GetFrontPtr()
        {
            Int32 front_temp = m_front + 1;
            if (front_temp == (Int32)Initial_Value.en_buffer_size)
                front_temp = 0;

            fixed (Byte* fix_buffer = m_buffer)
            {
                return (IntPtr)(fix_buffer + front_temp);
            }
        }

        unsafe public IntPtr GetRearPtr()
        {
            Int32 rear_temp = m_rear + 1;
            if (rear_temp == (Int32)Initial_Value.en_buffer_size)
                rear_temp = 0;

            fixed (Byte* fix_buffer = m_buffer)
            {
                return (IntPtr)(fix_buffer + rear_temp);
            }
        }
    }
}