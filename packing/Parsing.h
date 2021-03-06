#ifndef Parsing
#define Parsing

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#define Size 50

using namespace std;

enum STATUS
{
	find = 0,
	fail,
	exception
};

class PARSER
{
private:
	ifstream input;
	char *buf, temp[Size];
	int index, length, store;

	char* Get_Data();
	void Find_Str(const char *str);
	void Find_Exception(const char *str);
	vector<int> Make_Table(const char *pivot);
	int KMP(const char *pivot);

public:

	PARSER();
	~PARSER();

	void Init();
	void Find_Scope(const char *str);
	void Get_Value(const char *str, int &data);
	void Get_Value(const char *str, double &data);
	void Get_Value(const char *str, char &data);
	void Get_String(const char *str, char *data, int size);
};

#endif
