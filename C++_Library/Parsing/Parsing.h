#ifndef Parsing
#define Parsing
#define Bulk 255

#include <vector>
#include <fstream>
using namespace std;

namespace Olbbemi
{
   /*
	*  �������� ��������� �Լ��� �о�� ���ϸ� �ۼ� [ ������ �׻� .txt �̸� Unicode �������� ����Ǿ� �־�� �� ]
	*  ------------------------
	*  .txt ���� ���� ����  
	*  :Scope
	*  >
	*		string	=	data;
	*  ------------------------
	*  BOM ������ �ʱ� �ε�����ġ�� 2�� ����
	*  M_Find_Scope �Լ����� false ��ȯ : �ش� �������� ���ų� ��Ÿ�� �ǹ�
	*  ���ο� ������ Ž���� �ʿ��ϸ� M_Initialize �Լ� ȣ�� �� Ž��
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