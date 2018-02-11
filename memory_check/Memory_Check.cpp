#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <list>
#define Length 200

using namespace std;

class NODE
{
private:
	char path[Length];
	int size, line, check;
	void *ptr;

public:

	enum class ARRAY
	{
		single = 1,
		multi
	};

	enum class STATUS
	{
		FAIL = 0,
		NOALLOC,
		ARRAY,
		LEAK
	};

	friend class INFO;
	friend void Write(NODE &temp, int flag);
	friend void *operator new (size_t Size, const char *File, int Line);
	friend void *operator new[](size_t Size, const char *File, int Line);
	friend void operator delete (void *p);
	friend void operator delete[](void *p);

	NODE(void *ptr)
	{
		this->ptr = ptr;
	}

	NODE(int line, const char path[])
	{
		this->line = line;
		strcpy_s(this->path, path);
	}

	NODE(int size, int line, const char path[], void *ptr)
	{
		this->size = size;
		this->line = line;
		strcpy_s(this->path, path);
		this->ptr = ptr;
	}
};

class MANAGER
{
public:
	class LIST
	{
	public:
		NODE Node;
		LIST *next;
	};

	LIST *Head = NULL;

	~MANAGER()
	{
		LIST *temp = Head;
		while (temp != NULL)
		{
			Write(temp->Node, 3);
			temp = temp->next;
		}
	}
};

MANAGER Manager;

void Write(NODE &temp, int flag)
{
	FILE *output;
	char buf[Length], output_str[Length] = "Memory_";

	time_t info = time(NULL);
	struct tm local;
	localtime_s(&local, &info);

	sprintf_s(buf, sizeof(buf), "%4d%02d%02d_%02d%02d%02d", local.tm_year + 1900, local.tm_mon + 1, local.tm_yday + 1, local.tm_hour, local.tm_min, local.tm_sec);
	strcat_s(output_str, buf);	strcat_s(output_str, ".txt");

	fopen_s(&output, output_str, "a");

	switch (flag)
	{
	case (unsigned int)NODE::STATUS::FAIL:		fprintf_s(output, "FAIL						       %s : %d\n", temp.path, temp.line);	break;
	case (unsigned int)NODE::STATUS::NOALLOC:		fprintf_s(output, "NOALLOC			[0x%p]\n", temp.ptr);	break;
	case (unsigned int)NODE::STATUS::ARRAY:		fprintf_s(output, "ARRAY			[0x%p]  [     %d]  %s : %d\n", temp.ptr, temp.size, temp.path, temp.line);	break;
	case (unsigned int)NODE::STATUS::LEAK:		fprintf_s(output, "LEAK			[0x%p]  [     %d]  %s : %d\n", temp.ptr, temp.size, temp.path, temp.line);	break;
	}

	fclose(output);
}

void *operator new (size_t Size, const char *File, int Line)
{
	void *ptr = (void*)malloc(Size);

	if (ptr == NULL)
	{
		NODE temp(Line, File);
		Write(temp, (unsigned int)NODE::STATUS::FAIL);
		return NULL;
	}
	else
	{
		MANAGER::LIST *temp = (MANAGER::LIST*)malloc(sizeof(MANAGER::LIST));
		strcpy_s(temp->Node.path, File);
		temp->Node.ptr = ptr;	temp->Node.size = Size;	temp->Node.line = Line;
		temp->Node.check = (unsigned int)NODE::ARRAY::single; temp->next = NULL;

		if (Manager.Head == NULL)
			Manager.Head = temp;
		else
		{
			MANAGER::LIST *Pre = NULL, *Cur = Manager.Head;
			while (Cur != NULL)
			{
				Pre = Cur;
				Cur = Cur->next;
			}

			Pre->next = temp;
		}

		return ptr;
	}
}

void *operator new[](size_t Size, const char *File, int Line)
{
	void *ptr = (void*)malloc(Size);

	if (ptr == NULL)
	{
		NODE temp(Line, File);
		Write(temp, (unsigned int)NODE::STATUS::FAIL);
		return NULL;
	}
	else
	{
		MANAGER::LIST *temp = (MANAGER::LIST*)malloc(sizeof(MANAGER::LIST));
		strcpy_s(temp->Node.path, File);
		temp->Node.ptr = ptr;	temp->Node.size = Size;	temp->Node.line = Line;
		temp->Node.check = (unsigned int)NODE::ARRAY::multi;	temp->next = NULL;

		if (Manager.Head == NULL)
			Manager.Head = temp;
		else
		{
			MANAGER::LIST *Pre = NULL, *Cur = Manager.Head;
			while (Cur != NULL)
			{
				Pre = Cur;
				Cur = Cur->next;
			}

			Pre->next = temp;
		}

		return ptr;
	}
}

void operator delete (void *p)
{
	if (p == NULL)
		return;

	bool check = false;
	MANAGER::LIST *Pre = NULL, *Cur = Manager.Head;

	while (Cur != NULL && Cur->Node.ptr != p)
	{
		Pre = Cur;
		Cur = Cur->next;
	}

	if (Cur == NULL)
	{
		NODE temp(p);
		Write(temp, (unsigned int)NODE::STATUS::NOALLOC);
	}
	else
	{
		if (Cur->Node.check = (unsigned int)NODE::ARRAY::multi)
			Write(Cur->Node, (unsigned int)NODE::STATUS::ARRAY);

		if (Pre == NULL)
			Manager.Head = Cur->next;
		else
			Pre->next = Cur->next;

		Cur->next = NULL;
		free(p);	free(Cur);
	}
}

void operator delete[](void *p)
{
	if (p == NULL)
		return;

	bool check = false;
	MANAGER::LIST *Pre = NULL, *Cur = Manager.Head;

	while (Cur != NULL && Cur->Node.ptr != p)
	{
		Pre = Cur;
		Cur = Cur->next;
	}

	if (Cur == NULL)
	{
		NODE temp(p);
		Write(temp, (unsigned int)NODE::STATUS::NOALLOC);
	}
	else
	{
		if (Cur->Node.check = (unsigned int)NODE::ARRAY::single)
			Write(Cur->Node, (unsigned int)NODE::STATUS::ARRAY);

		if (Pre == NULL)
			Manager.Head = Cur->next;
		else
			Pre->next = Cur->next;

		Cur->next = NULL;
		free(p);	free(Cur);
	}
}

//// 컴파일 에러를 막기 위해 선언
void operator delete (void * p, char *File, int Line) {}
void operator delete[](void * p, char *File, int Line) {}
