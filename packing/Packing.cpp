#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "header.h"
#define Length 30

// 파서에서 불러올 정보 define
#define SCOPE ":INFO"
#define CNT "cnt"
#define FILE1 "file1"
#define FILE2 "file2"
#define FILE3 "file3"
// 여기까지

struct SubHeader
{
	char FileName[Length];
	int FileSize, Offset;
};

struct MainHeader
{
	char directive[Length], New[Length];
	int num;
};

FILE *input, *output;
PARSER parsing;

int main()
{
	MainHeader Header;
	SubHeader *Sub;

	char str[Length], *buf;
	int cnt, *info, total = 0;

	// 패킹에 필요한 Scope 찾기
	parsing.Find_Scope(SCOPE);
	parsing.Get_Value(CNT, cnt);

	strcpy_s(Header.directive, sizeof(Header.directive), "OLBBEMI");
	Header.num = cnt;

	info = (int*)malloc(sizeof(int) * cnt);
	Sub = (SubHeader*)malloc(sizeof(SubHeader) * cnt);

	// 패킹 원하는 파일 나열
	parsing.Get_String(FILE1, str, Length);
	strcpy_s(Sub[0].FileName, sizeof(Sub[0].FileName), str);

	parsing.Get_String(FILE2, str, Length);
	strcpy_s(Sub[1].FileName, sizeof(Sub[1].FileName), str);

	parsing.Get_String(FILE3, str, Length);
	strcpy_s(Sub[2].FileName, sizeof(Sub[2].FileName), str);
	// 여기까지

	for (int i = 0; i < cnt; i++)
		Sub[i].Offset = 0;

	printf("패킹 파일 이름을 적으시오 : ");	
	scanf_s("%s", Header.New, sizeof(Header.New));

	fopen_s(&output, Header.New, "wb");
	fwrite(&Header, sizeof(Header), 1, output);

	total += sizeof(Header);
	for (int i = 0; i < cnt; i++)
	{
		fopen_s(&input, Sub[i].FileName, "rb");
		fseek(input, 0, SEEK_END);
		Sub[i].FileSize = ftell(input);
		total += sizeof(Sub[i]);
		fclose(input);
	}

	for (int i = 0; i < cnt; i++)
	{
		fopen_s(&input, Sub[i].FileName, "rb");
		Sub[i].Offset = total;

		fseek(input, 0, SEEK_END);
		info[i] = ftell(input);
		total += info[i];

		fwrite(&Sub[i], sizeof(Sub[i]), 1, output);
		fclose(input);
	}

	for (int i = 0; i < cnt; i++)
	{
		fopen_s(&input, Sub[i].FileName, "rb");

		buf = (char*)malloc(sizeof(char) * info[i]);
		fread(buf, info[i], 1, input);
		fwrite(buf, info[i], 1, output);

		free(buf);
		fclose(input);
	}

	fclose(output);
	free(Sub);	free(info);
	return 0;
}
