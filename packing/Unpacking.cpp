#include <cstdio>
#include <cstdlib>
#include <cstring>
#define Length 30

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

int main()
{
	MainHeader *buf;
	SubHeader *Sub;

	char str[Length], *temp;

	printf("언패킹 파일이름을 적으시오 : ");	scanf_s("%s", str, sizeof(str));
	fopen_s(&output, str, "rb");

	buf = (MainHeader*)malloc(sizeof(MainHeader) * Length);
	fread(buf, sizeof(MainHeader), 1, output);

	if (strcmp(buf->directive, "OLBBEMI"))
	{
		printf("잘못 패킹된 파일입니다.\n");
		return 0;
	}
	else
	{
		Sub = (SubHeader*)malloc(sizeof(SubHeader) * buf->num);

		for (int i = 0; i < buf->num; i++)
			fread(&Sub[i], sizeof(SubHeader), 1, output);

		for (int i = 0; i < buf->num; i++)
		{
			fseek(output, Sub[i].Offset, SEEK_SET);
			temp = (char*)malloc(sizeof(char) * Sub[i].FileSize);
			fread(temp, Sub[i].FileSize, 1, output);

			fopen_s(&input, Sub[i].FileName, "wb");
			fwrite(temp, Sub[i].FileSize, 1, input);
			fclose(input);
		}
	}

	fclose(output);
	return 0;
}
