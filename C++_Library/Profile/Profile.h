#ifndef Profile
#define Profile
#define MICRO 1000000.0

#include <Windows.h>
#include <tchar.h>

#include <vector>
using namespace std;

namespace Olbbemi
{
	#define Profiler
	#ifdef Profiler
		#define BEGIN(Name, LINE)	Profile_Begin(Name, LINE)
		#define END(Name)			Profile_End(Name)
	#else
		#define BEGIN(None)
		#define END(None)
	#endif

	class C_Profile;

	class C_Profile_Chunk
	{
	private:
		class C_Node
		{
			TCHAR m_function_name[25];
			int m_min_count, m_max_count;

			__int64 m_min_value[3], m_max_value[3], m_call_count, m_start_time, m_total_time;

		public:
			C_Node(const PTCHAR pa_str, __int64 pa_time);

			friend class C_Profile;
			friend class C_Profile_Chunk;
			
			friend void Profile_Begin(const PTCHAR pa_str, int pa_line);
			friend void Profile_End(const PTCHAR pa_str);
		};

		enum class E_Status
		{
			normal = 0,
			fail,
			success
		};

		DWORD m_thread_id;
		LARGE_INTEGER m_begin_count, m_end_count;
		vector<C_Node*> m_node_list;

		static DWORD s_tls_index;

		friend class C_Profile;

	public:

		C_Profile_Chunk();
		~C_Profile_Chunk();
		
		friend void Profile_Begin(const PTCHAR p_str, int p_line);
		friend void Profile_End(const PTCHAR p_str);
	};

	class C_Profile
	{
	private:
		bool m_is_lock_on;
		TCHAR m_file[30];
		LONG m_thread_count;
		LARGE_INTEGER m_frequency;

		static LONG s_index;
		static C_Profile_Chunk** s_chunk; 

		friend class C_Profile_Chunk;

	public:
		C_Profile(int pa_thread_count);
		~C_Profile();

		void M_Save();
		void M_GetTime();

		static C_Profile_Chunk* S_Alloc();
	};

	void Profile_Begin(const PTCHAR pa_str, int pa_line);
	void Profile_End(const PTCHAR pa_str);
}

#endif