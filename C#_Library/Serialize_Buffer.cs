using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace SERIALIZE_BUFFER
{
    class SerializeBuffer
    {
        private byte[] m_buffer;
        private Int32 m_front, m_rear;

        private enum Initial_Value : Int32
        {
            en_buffer_size = 100
        }


        /*
            직렬화버퍼 초기화 및 사용할 패킷의 헤더 크기만큼 공간 비워두는 용도로 정의 
        */
        public SerializeBuffer(Int32 p_header_size)
        {
            m_buffer = new byte[(Int32)Initial_Value.en_buffer_size];
            m_front = m_rear = p_header_size;
        }


        /*
             큐의 전체 사용 및 미사용 크기 얻는 함수
        */
        public Int32 GetUsingSize()
        {
            return m_rear - m_front;
        }

        public Int32 GetUnusingSize()
        {
            return (Int32)Initial_Value.en_buffer_size - m_rear - 1;
        }


        /*
            Enqueue 및 Dequeue 함수 이외 외적으로 버퍼의 front 및 rear 정보가 변경되어야할 경우 호출되는 함수 
        */
        public void MoveFront(Int32 p_size)
        {
            m_front += p_size;
        }

        public void MoveRear(Int32 p_size)
        {
            m_rear += p_size;
        }

        /*
            패킷 생성시 페이로드 삽입이후 헤더부분을 추가하기 위한 함수 
        */
        public void MakeHeader(Byte[] p_input_data, Int32 p_size)
        {
            m_front = 0;
            System.Array.Copy(p_input_data, m_buffer, p_size);
        }


        /*
            버퍼의 시작주소를 반환하는 함수
        */
        unsafe public IntPtr GetBufferPtr()
        {
            fixed (Byte* fix_buffer = m_buffer)
                return (IntPtr)(fix_buffer + m_front);
        }


        /*
            버퍼에 데이터를 삽입 및 삭제하는 함수 
        */
        unsafe public bool Enqueue(IntPtr p_input_data, Int32 p_size)
        {
            fixed (Byte* fix_buffer = m_buffer)
            {
                switch (p_size)
                {
                    case 1: *(Byte*)(fix_buffer + m_rear) = *(Byte*)(p_input_data); break;
                    case 2: *(Int16*)(fix_buffer + m_rear) = *(Int16*)(p_input_data); break;
                    case 4: *(Int32*)(fix_buffer + m_rear) = *(Int32*)(p_input_data); break;
                    case 8: *(Int64*)(fix_buffer + m_rear) = *(Int64*)(p_input_data); break;
                    default: System.Runtime.InteropServices.Marshal.Copy(p_input_data, m_buffer, m_rear, p_size); break;
                }
            }

            m_rear += p_size;
            return true;
        }

        unsafe public bool Dequeue(IntPtr p_output_data, Int32 p_size)
        {
            if (GetUsingSize() < p_size)
                return false;

            fixed (Byte* fix_buffer = m_buffer)
            {
                switch (p_size)
                {
                    case 1: *(Byte*)(p_output_data) = *(Byte*)(fix_buffer + m_front); break;
                    case 2: *(Int16*)(p_output_data) = *(Int16*)(fix_buffer + m_front); break;
                    case 4: *(Int32*)(p_output_data) = *(Int32*)(fix_buffer + m_front); break;
                    case 8: *(Int64*)(p_output_data) = *(Int64*)(fix_buffer + m_front); break;
                    default: System.Runtime.InteropServices.Marshal.Copy(m_buffer, m_front, p_output_data, p_size); break;
                }
            }

            m_front += p_size;
            return true;
        }
    }
}