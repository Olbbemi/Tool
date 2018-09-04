#include "Precompile.h"
#include "Parsing.h"

#include <locale>

#include <string.h>
#include <strsafe.h>

using namespace Olbbemi;

C_Parser::C_Parser() : m_index(1)
{
	wifstream lo_read_stream;

	lo_read_stream.open(_TEXT("ChatServer.txt"), ios_base::binary);
	lo_read_stream.imbue(locale(locale(""), new Unicodecvt));
	lo_read_stream.seekg(0, lo_read_stream.end);

	m_length = (int)lo_read_stream.tellg();
	lo_read_stream.seekg(0, lo_read_stream.beg);

	m_buffer = new TCHAR[m_length * sizeof(TCHAR)];
	lo_read_stream.read(m_buffer, m_length);

	lo_read_stream.close();
}

void C_Parser::M_Initialize()
{
	m_index = m_index_store = 1;
}

void C_Parser::M_Make_KMP_Table(const TCHAR* pa_pivot)
{
	size_t lo_size = _tcslen(pa_pivot), i = 1;
	for (int j = 0; i < lo_size; i++)
	{
		while (j > 0 && pa_pivot[i] != pa_pivot[j])
			j = m_KMP_table[j - 1];

		if (pa_pivot[i] == pa_pivot[j])
			m_KMP_table[i] = ++j;
	}
}

C_Parser::E_Status C_Parser::M_KMP_Algorithm(const TCHAR* pa_pivot)
{
	size_t lo_pivot_size = _tcslen(pa_pivot), lo_buffer_size = _tcslen(m_buffer), j = 0;
	for (int i = m_index; i < lo_buffer_size; i++)
	{
		if (m_buffer[i] == '/' && m_buffer[m_index + 1] == '/' || (m_buffer[m_index - 1] == '/' && m_buffer[m_index] == '/'))
		{
			m_index = i;
			return E_Status::exception;
		}

		while (j > 0 && pa_pivot[j] != m_buffer[i])
			j = m_KMP_table[j - 1];

		if (m_buffer[i] == pa_pivot[j])
		{
			if (j == lo_pivot_size - 1)
			{
				m_index = i + 1;
				return E_Status::find;
			}
			else
				j++;
		}
	}

	return E_Status::fail;
}

void C_Parser::M_Find_Str(const TCHAR* pa_str)
{
	m_KMP_table.resize(_tcslen(pa_str));
	fill(m_KMP_table.begin(), m_KMP_table.end(), 0);
	
	M_Make_KMP_Table(pa_str);

	m_index = m_index_store;
	while (1)
	{
		if ((m_buffer[m_index] == '/' && m_buffer[m_index + 1] == '/') || (m_buffer[m_index - 1] == '/' && m_buffer[m_index] == '/'))
			M_Find_Symbol(_TEXT("//"));
		else if (m_buffer[m_index] == '/' && m_buffer[m_index + 1] == '*')
			M_Find_Symbol(_TEXT("/*"));
		else if (m_buffer[m_index] == '\'' || m_buffer[m_index] == '\"' || m_buffer[m_index] == '\n')
			m_index++;
		else
		{
			E_Status lo_return_value = M_KMP_Algorithm(pa_str);
			if (lo_return_value == E_Status::find)
				break;
		}

		if (m_index == m_length)
			break;
	}
}

bool C_Parser::M_Find_Scope(const TCHAR* pa_str)
{
	m_KMP_table.resize(_tcslen(pa_str));
	fill(m_KMP_table.begin(), m_KMP_table.end(), 0);
	
	M_Make_KMP_Table(pa_str);

	while (1)
	{
		if ((m_buffer[m_index] == '/' && m_buffer[m_index + 1] == '/') || (m_buffer[m_index - 1] == '/' && m_buffer[m_index] == '/'))
			M_Find_Symbol(_TEXT("//"));
		else if (m_buffer[m_index] == '/' && m_buffer[m_index + 1] == '*')
			M_Find_Symbol(_TEXT("/*"));
		else if (m_buffer[m_index] == '\'' || m_buffer[m_index] == '\"' || m_buffer[m_index] == '\n')
			m_index++;
		else
		{
			E_Status lo_return_value = M_KMP_Algorithm(pa_str);
			if (lo_return_value == E_Status::find)
			{
				m_index += 5;
				m_index_store = m_index;
				break;
			}
			else if(lo_return_value == E_Status::fail)
			{
				m_index = 0;
				return false;
			}
		}

		if (m_index == m_length)
			break;
	}

	return true;
}

void C_Parser::M_Find_Symbol(const TCHAR* pa_str)
{
	if (!_tcscmp(pa_str,_TEXT("//")))
	{
		while (m_buffer[m_index] != '\n')
			m_index++;
		m_index++;
	}
	else if (!_tcscmp(pa_str,_TEXT("/*")))
	{
		while (m_buffer[m_index - 1] != '*' || m_buffer[m_index] != '/')
			m_index++;
		m_index++;
	}
}

TCHAR* C_Parser::M_Get_Data()
{
	while (m_buffer[m_index] != '=')
		m_index++;
	m_index += 2;

	int i = 0;
	for (; m_buffer[m_index] != ';'; m_index++)
	{
		if (m_buffer[m_index] == '\"' || m_buffer[m_index] == '\'')
			continue;

		m_return_value[i++] = m_buffer[m_index];
	}

	m_return_value[i] = '\0';
	return m_return_value;
}

void C_Parser::M_Get_Value(const TCHAR* pa_str, int& pa_data)
{ 
	M_Find_Str(pa_str);
	pa_data = _ttoi(M_Get_Data());
}

void C_Parser::M_Get_Value(const TCHAR* pa_str, double& pa_data)
{
	M_Find_Str(pa_str);
	pa_data = _ttof(M_Get_Data());
}

void C_Parser::M_Get_Value(const TCHAR* pa_str, TCHAR& pa_data)
{
	M_Find_Str(pa_str);
	TCHAR *temp = M_Get_Data();
	pa_data = temp[0];
}

void C_Parser::M_Get_String(const TCHAR* pa_str, TCHAR* pa_data, int pa_size)
{
	M_Find_Str(pa_str);
	StringCchCopy(pa_data, pa_size, M_Get_Data());
}