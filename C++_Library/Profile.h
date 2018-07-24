#ifndef Profile
#define Profile
#define MICRO 1000000.0
#define Profiler

#ifdef Profiler
	#define BEGIN(Obj, Name, LINE)	Profile_Begin(Obj, Name, LINE)
	#define END(Obj, Name)			Profile_End(Obj, Name)
#else
	#define BEGIN(None)
	#define END(None)
#endif

namespace Olbbemi 
{
	class PROFILE
	{
	private:
		class NODE
		{
			TCHAR m_function_name[25];
			int m_min_count, m_max_count;

			_int64 m_min_value[3], m_max_value[3], m_call_count, m_start_time, m_total_time;

		public:
			NODE(const PTCHAR p_str, _int64 p_time);

			friend class PROFILE;
			friend void Profile_Begin(PROFILE *p_obj, const PTCHAR p_str, int p_line);
			friend void Profile_End(PROFILE *p_obj, const PTCHAR p_str);
		};

		enum class STATUS
		{
			e_Normal = 0,
			e_Fail,
			e_Success
		};

		bool m_is_lock;
		TCHAR m_file[30];
		LARGE_INTEGER m_frequency, begin_count, end_count;

		list<NODE*> node_list;

		void GetTime();

	public:
		PROFILE();
		void Save();
		void Delete();

		friend void Profile_Begin(PROFILE *p_obj, const PTCHAR p_str, int p_line);
		friend void Profile_End(PROFILE *p_obj, const PTCHAR p_str);
	};
}

#endif