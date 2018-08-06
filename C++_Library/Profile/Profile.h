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

#include <vector>
using namespace std;

namespace Olbbemi
{
	/*
	 *  �������ϸ��� �ʿ��� ��ġ���� C_Profile ��ü ���� �� BEGIN �� END ��ũ�θ� �̿� [ BEGIN �� END ��ũ�δ� �� ������ ���� ]
	 *  M_Save() �Լ�: �������ϸ��� ������ ���Ϸ� �����ϴ� �Լ� [ Home Key ]
	 *  M_Delete() �Լ�: ������� �������ϸ��� ������ �����ϴ� �Լ� [ End Key ]
	 *  U Key: Ű �Է� Ȱ��ȭ, L Key: Ű �Է� ��Ȱ��ȭ
	 */

	class C_Profile
	{
	private:
		class C_Node
		{
			TCHAR m_function_name[25];
			int m_min_count, m_max_count;

			_int64 m_min_value[3], m_max_value[3], m_call_count, m_start_time, m_total_time;

		public:
			C_Node(const PTCHAR pa_str, __int64 pa_time);

			friend class C_Profile;
			friend void Profile_Begin(C_Profile *pa_obj, const PTCHAR pa_str, int pa_line);
			friend void Profile_End(C_Profile *pa_obj, const PTCHAR pa_str);
		};

		enum class E_Status
		{
			normal = 0,
			fail,
			success
		};

		bool m_is_lock_on;
		TCHAR m_file[30];
		LARGE_INTEGER m_frequency, m_begin_count, m_end_count;

		vector<C_Node*> m_node_list;

		void M_GetTime();

	public:
		C_Profile();
		void M_Save();
		void M_Delete();

		friend void Profile_Begin(C_Profile *p_obj, const PTCHAR p_str, int p_line);
		friend void Profile_End(C_Profile *p_obj, const PTCHAR p_str);
	};
}


#endif