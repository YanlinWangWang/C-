#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>

class MemoryAlloc;
//�ڴ�� ��С��Ԫ
class MemoryBlock
{
public:
	//�������ڴ�飨�أ�
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	char c1;
	char c2;
	char c3;
};

//��������ڴ�
class MemoryAlloc
{
public:
	//��ʼ������
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSzie = 0;
		_nBlockSzie = 0;
	}

	~MemoryAlloc()
	{

	}


	//�����ڴ�
	//����nSize��С���ڴ�
	//һ�����������������˶�Ӧ�ڴ�ؿռ�
	void* allocMem(size_t nSize)
	{
		//û�г�ʼ����ô�ͳ�ʼ���ڴ����
		if (!_pBuf)
		{
			initMemory();
		}

		//pReturn����ָ��
		MemoryBlock* pReturn = nullptr;
		//�ж��ڴ��������û��
		if (nullptr == _pHeader)
		{
			//������ô��Ŀռ� ���һ�Ҫ����ͷ�ڵ�
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 0;
			pReturn->pAlloc = this;
			pReturn->pNext = nullptr;
		}
		//Ҫ���ڴ�����滹����ô��ͷָ����һ���ڵ�
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		//�����ж����ڴ�ػ�������������ڴ�
		//���ڴ��Ŀ�ͷ
		/*
		��ַ�ṹ����
		�ڴ��ͷ�ļ�      ����ʹ�õĵ�ַ
		                  ^1
						  ����ط��Ǵ���ĵ�ַҲ����pMem
		����Ҫ��ȥ(MemoryBlock)��С��λ�� ����ָ��ͷ�ڵ�
		*/
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));

		//��Ϊÿ���ڵ㶼ֻ�ᱻʹ��һ��
		assert(pBlock->nRef == 1);

		//�����������Ϊ1��ôΪ0
		if (--pBlock->nRef != 0)
		{
			return;
		}

		//�����Ǵ��ڴ�������뵽���ڴ滹��ֱ�Ӵ�OS����������ڴ�
		if (pBlock->bPool)//���ڴ�ص���
		{
			//��ô����ֱ�Ӳ��뵽�ڴ��������ĵ�һ��
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		//������ôֱ���ͷ�
		else
		{
			free(pMem);
		}

	}

	//��ʼ��
	void initMemory()
	{	//����
		assert(nullptr == _pBuf);
		if (!_pBuf)
			return;
		//�����ڴ�صĴ�С
		size_t bufSize = _nSzie * _nBlockSzie;
		//��ϵͳ����ص��ڴ�
		_pBuf = (char*)malloc(bufSize);

		//��ʼ���ڴ��
		//���ȳ�ʼ����һ���ڴ� ��Ҳ�����ڴ�����������ͷ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;

		//�����ڴ����г�ʼ��
		//Ȼ�����ʣ�µ�n���ڴ���б���
		//���γ�ʼ����Ӧ�ڵ� ���ҽ��ڵ���뵽��Ӧ���ڴ����������ȥ
		MemoryBlock* pTemp1 = _pHeader;
		for (size_t n = 1; n < _nBlockSzie; n++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * _nSzie));
			pTemp2->bPool = true;
			pTemp2->nID = 0;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
private:
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ�Ĵ�С
	size_t _nSzie;
	//�ڴ浥Ԫ������
	size_t _nBlockSzie;
};
#endif