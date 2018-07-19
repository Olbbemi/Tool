using System;
using System.Text;
using System.Diagnostics;

namespace LOG
{
    /*
         로그를 남길 파일에 필요한 헤더 및 함수
         1. using System.Diagnostics
         2. StackFrame 객체명 = new StackFrame(true);
         3. Log.WriteLog((Byte)Log.LOG_STATE.로그레벨, 객체명.GetFileLineNumber(), "sample");
            
            => 3번의 GetFileLineNumber() 함수가 반환하는 값은 2번이 할당된 위치이므로 로그를 남길때마다 2번을 호출해주어야 함
    */

    static class Log
    {
        public enum LOG_STATE : Byte
        {
            e_error = 1,
            e_warning,
            e_debug
        }

        static public Byte m_log_level;
        static private string m_log_name;

        static Log()
        {
            m_log_level = (Byte)LOG_STATE.e_debug;

            m_log_name = DateTime.Now.ToString("yyyyMMdd");
            m_log_name += "_Log.txt";

        }

        static public void WriteLog(Byte p_log_level, Int32 p_line, string p_log)
        {
            if (m_log_level < p_log_level)
                return;

            string body;

            body = "---------------------------------------------------------" + System.Environment.NewLine;
            switch (m_log_level)
            {
                case (Byte)LOG_STATE.e_error:
                    body += "[ERROR]_Line :" + p_line.ToString() + System.Environment.NewLine;
                    break;

                case (Byte)LOG_STATE.e_warning:
                    body += "[WARNING]_Line :" + p_line.ToString() + System.Environment.NewLine;
                    break;

                case (Byte)LOG_STATE.e_debug:
                    body += "[DEBUG]_Line :" + p_line.ToString() + System.Environment.NewLine;
                    break;
            }

            body += p_log + System.Environment.NewLine;
            body += "---------------------------------------------------------" + System.Environment.NewLine;

            System.IO.FileInfo file = new System.IO.FileInfo(m_log_name);
            if (file.Exists == true)
                System.IO.File.AppendAllText(m_log_name, body, Encoding.Default);
            else
                System.IO.File.WriteAllText(m_log_name, body, Encoding.Default);
        }
    }
}