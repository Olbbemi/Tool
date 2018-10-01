#ifndef Parsing
#define Parsing
#define Bulk 255

#include <vector>
#include <fstream>
using namespace std;

namespace Olbbemi
{
   /*
	*  생성자의 파일입출력 함수에 읽어올 파일명 작성 [ 파일은 항상 .txt 이며 Unicode 형식으로 저장되어 있어야 함 ]
	*  ------------------------
	*  .txt 파일 내부 형식  
	*  :Scope
	*  >
	*		string	=	data;
	*  ------------------------
	*  BOM 때문에 초기 인덱스위치를 2로 설정
	*  M_Find_Scope 함수에서 false 반환 : 해당 스코프가 없거나 오타를 의미
	*  새로운 스코프 탐색이 필요하면 M_Initialize 함수 호출 후 탐색
	*/

	class C_Parser
	{
	private:
		enum class E_Status : char
		{
			find = 0,
			fail,
			exception
		};

		TCHAR *m_buffer, m_return_value[Bulk], m_log_action[20], m_log_server[20];
		int m_index, m_length, m_index_store;
		vector<int> m_KMP_table;

		TCHAR* M_Get_Data();
		void M_Find_Str(const TCHAR* pa_str);
		void M_Find_Symbol(const TCHAR* pa_str);
		void M_Make_KMP_Table(const TCHAR* pa_pivot);
		E_Status M_KMP_Algorithm(const TCHAR* pa_pivot);

	public:
		C_Parser(TCHAR* pa_path);

		void M_Initialize();
		void M_Find_Scope(const TCHAR* pa_str);
		void M_Get_Value(const TCHAR* pa_str, int& pa_data);
		void M_Get_Value(const TCHAR* pa_str, double& pa_data);
		void M_Get_Value(const TCHAR* pa_str, TCHAR& pa_data);
		void M_Get_String(const TCHAR* pa_str, TCHAR* pa_data, int pa_size);
		void M_Get_String(const TCHAR* pa_str, char* pa_data, int pa_size);
	};

	class Unicodecvt : public codecvt<wchar_t, char, mbstate_t>
	{
	protected:
		virtual bool __CLR_OR_THIS_CALL do_always_noconv() const noexcept
		{
			return true;
		}
	};
}

#endif