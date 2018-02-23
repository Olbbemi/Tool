#include <string.h>
#include "Parsing.h"
#include "Precompile.h"

PARSER::PARSER() : index(0)
{
	input.open(_TEXT("Bitmap_Data.txt"), ios_base::binary);
	input.imbue(locale(locale(""), new Unicodecvt));
	input.seekg(0, input.end);

	length = (int)input.tellg();
	input.seekg(0, input.beg);

	buf = new TCHAR[length * sizeof(TCHAR)];
	input.read(buf, length);
}

PARSER::~PARSER()
{
	input.close();
}

void PARSER::Init()
{
	index = store = 0;
}

void PARSER::Make_Table(const TCHAR *pivot)
{
	size_t size = _tcslen(pivot), i = 1;
	for (int j = 0; i < size; i++)
	{
		while (j > 0 && pivot[i] != pivot[j])
			j = table[j - 1];

		if (pivot[i] == pivot[j])
			table[i] = ++j;
	}
}

int PARSER::KMP(const TCHAR *pivot)
{
	size_t Psize = _tcslen(pivot), Bsize = _tcslen(buf), j = 0;
	for (int i = index; i < Bsize; i++)
	{
		if (buf[i] == '/' && buf[index + 1] == '/' || (buf[index - 1] == '/' && buf[index] == '/'))
		{
			index = i;
			return STATUS::exception;
		}

		while (j > 0 && pivot[j] != buf[i])
			j = table[j - 1];

		if (buf[i] == pivot[j])
		{
			if (j == Psize - 1)
			{
				index = i + 1;
				return STATUS::find;
			}
			else
				j++;
		}
	}

	return STATUS::fail;
}

void PARSER::Find_Str(const TCHAR *str)
{
	table.resize(_tcslen(str));
	fill(table.begin(), table.end(), 0);
	
	Make_Table(str);

	index = store;
	while (1)
	{
		if ((buf[index] == '/' && buf[index + 1] == '/') || (buf[index - 1] == '/' && buf[index] == '/'))
			Find_Exception(_TEXT("//"));
		else if (buf[index] == '/' && buf[index + 1] == '*')
			Find_Exception(_TEXT("/*"));
		else if (buf[index] == '\'' || buf[index] == '\"' || buf[index] == '\n')
			index++;
		else
		{
			int temp = KMP(str);

			if (temp == STATUS::find)
				break;
		}

		if (index == length)
			break;
	}
}

void PARSER::Find_Scope(const TCHAR *str)
{
	table.resize(_tcslen(str));
	fill(table.begin(), table.end(), 0);
	
	Make_Table(str);

	while (1)
	{
		if ((buf[index] == '/' && buf[index + 1] == '/') || (buf[index - 1] == '/' && buf[index] == '/'))
			Find_Exception(_TEXT("//"));
		else if (buf[index] == '/' && buf[index + 1] == '*')
			Find_Exception(_TEXT("/*"));
		else if (buf[index] == '\'' || buf[index] == '\"' || buf[index] == '\n')
			index++;
		else
		{
			int temp = KMP(str);

			if (temp == STATUS::find)
			{
				index += 5;
				store = index;
				break;
			}
			else if(temp == STATUS::fail)
			{
				index = 0;
				break;
			}
		}

		if (index == length)
			break;
	}
}

void PARSER::Find_Exception(const TCHAR *str)
{
	if (!_tcscmp(str,_TEXT("//")))
	{
		while (buf[index] != '\n')
			index++;
		index++;
	}
	else if (!_tcscmp(str,_TEXT("/*")))
	{
		while (buf[index - 1] != '*' || buf[index] != '/')
			index++;
		index++;
	}
}

TCHAR* PARSER::Get_Data()
{
	while (buf[index] != '=')
		index++;
	index += 2;

	int i = 0;
	for (; buf[index] != ';'; index++)
	{
		if (buf[index] == '\"' || buf[index] == '\'')
			continue;

		temp[i++] = buf[index];
	}

	temp[i] = '\0';
	return temp;
}

void PARSER::Get_Value(const TCHAR *str, int &data)
{
	Find_Str(str);
	data = _ttoi(Get_Data());
}

void PARSER::Get_Value(const TCHAR *str, double &data)
{
	Find_Str(str);
	data = _ttof(Get_Data());
}

void PARSER::Get_Value(const TCHAR *str, TCHAR &data)
{
	Find_Str(str);
	TCHAR *temp = Get_Data();
	data = temp[0];
}

void PARSER::Get_String(const TCHAR *str, TCHAR *data, int size)
{
	Find_Str(str);
	_tcscpy_s(data, size, Get_Data());
}
