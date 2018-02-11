#include <string.h>
#include "Parsing.h"

PARSER::PARSER() : index(0)
{
	input.open("File_Name"); 
	input.seekg(0, input.end);

	length = (int)input.tellg();
	input.seekg(0, input.beg);

	buf = new char[length];
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

void PARSER::Make_Table(const char *pivot)
{
	int size = strlen(pivot);
	for (int i = 1, j = 0; i < size; i++)
	{
		while (j > 0 && pivot[i] != pivot[j])
			j = table[j - 1];

		if (pivot[i] == pivot[j])
			table[i] = ++j;
	}
}

int PARSER::KMP(const char *pivot)
{
	int Psize = strlen(pivot), Bsize = strlen(buf);
	for (int i = index, j = 0; i < Bsize; i++)
	{
		if (buf[i] == '/')
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

void PARSER::Find_Str(const char *str)
{
	table.resize(strlen(str));
	fill(table.begin(), table.end(), 0);
	
	Make_Table(str);

	index = store;
	while (1)
	{
		if ((buf[index] == '/' && buf[index + 1] == '/') || (buf[index - 1] == '/' && buf[index] == '/'))
			Find_Exception("//");
		else if (buf[index] == '/' && buf[index + 1] == '*')
			Find_Exception("/*");
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

void PARSER::Find_Scope(const char *str)
{
	table.resize(strlen(str));
	fill(table.begin(), table.end(), 0);
	
	Make_Table(str);

	while (1)
	{
		if ((buf[index] == '/' && buf[index + 1] == '/') || (buf[index - 1] == '/' && buf[index] == '/'))
			Find_Exception("//");
		else if (buf[index] == '/' && buf[index + 1] == '*')
			Find_Exception("/*");
		else if (buf[index] == '\'' || buf[index] == '\"' || buf[index] == '\n')
			index++;
		else
		{
			int temp = KMP(str);

			if (temp == STATUS::find)
			{
				index += 2;
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

void PARSER::Find_Exception(const char *str)
{
	if (!strcmp(str,"//"))
	{
		while (buf[index] != '\n')
			index++;
		index++;
	}
	else if (!strcmp(str,"/*"))
	{
		while (buf[index - 1] != '*' || buf[index] != '/')
			index++;
		index++;
	}
}

char* PARSER::Get_Data()
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

void PARSER::Get_Value(const char *str, int &data)
{
	Find_Str(str);
	data = atoi(Get_Data());
}

void PARSER::Get_Value(const char *str, double &data)
{
	Find_Str(str);
	data = atof(Get_Data());
}

void PARSER::Get_Value(const char *str, char &data)
{
	Find_Str(str);
	char* temp = Get_Data();
	data = temp[0];
}

void PARSER::Get_String(const char *str, char *data, int size)
{
	Find_Str(str);
	strcpy_s(data, size, Get_Data());
}
