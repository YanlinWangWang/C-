#include <iostream>
using namespace std;

int main()
{
	//128byte
	char* data1 = new char[128];
	delete[] data1;

	//�ڶ������ظ�ʽ
	char* data2 = new char;
	delete data2;

	char* data3 = (char*)malloc(64);
	free(data3);

	return 0;
}