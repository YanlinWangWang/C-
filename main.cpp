
#include<stdlib.h>
#include<iostream>
#include<thread>
#include<mutex>//��
#include"CELLTimestamp.h"
using namespace std;
//ԭ�Ӳ���   ԭ�� ���� 
mutex m;
const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;

void workFun(int index)
{
	char* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[(rand() % 128) + 1];
	}
	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
	/*
	for (int n = 0; n < nCount; n++)
	{
		//�Խ���
		//lock_guard<mutex> lg(m);
		//�ٽ�����-��ʼ
		//m.lock();

		//m.unlock();
		//�ٽ�����-����
	}//�̰߳�ȫ �̲߳���ȫ
	 //ԭ�Ӳ��� �������������ʱ��С�Ĳ�����λ
	 */
}//��ռʽ
int main()
{
	//1
	char* data1 = new char[128];
	delete[] data1;
	//2
	char* data2 = new char;
	delete data2;
	//3
	char* data3 = new char[64];
	delete[] data3;


	char* data[10000];
	for (size_t i = 0; i < 10000; i++)
	{
		data[i] = new char[1 + i];
	}
	for (size_t i = 0; i < 10000; i++)
	{
		delete[] data[i];
	}

	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] = thread(workFun, n);
	}
	CELLTimestamp tTime;
	for (int n = 0; n < tCount; n++)
	{
		t[n].join();
		//t[n].detach();
	}
	cout << tTime.getElapsedTimeInMilliSec() << endl;
	cout << "Hello,main thread." << endl;
	return 0;
	return 0;
}