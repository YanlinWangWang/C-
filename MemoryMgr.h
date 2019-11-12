#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>
#include <mutex>

//Ĭ�ϴ�С
#define MAX_MEMORY_SZIE 1024

#ifdef _DEBUG
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif // _DEBUG

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
		_nSize = 0;
		_nBlockSize = 0;
	}

	~MemoryAlloc()
	{
		if (_pBuf)
			free(_pBuf);
	}


	//�����ڴ�
	//����nSize��С���ڴ�
	//һ�����������������˶�Ӧ�ڴ�ؿռ�
	void* allocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
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
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		//Ҫ���ڴ�����滹����ô��ͷָ����һ���ڵ�
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		//�����ӡ
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//�ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		std::lock_guard<std::mutex> lg(_mutex);
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
			free(pBlock);
		}

	}

	//��ʼ��
	void initMemory()
	{	//����
		assert(nullptr == _pBuf);
		if (_pBuf)//�վ�ֱ�ӷ���
			return;
		//�����ڴ�صĴ�С
		size_t realSzie = _nSize + sizeof(MemoryBlock);//�����Ĵ�С 
		size_t bufSize = realSzie * _nBlockSize;
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
		for (size_t n = 1; n < _nBlockSize; n++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
protected:
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ�Ĵ�С
	size_t _nSize;
	//�ڴ浥Ԫ������
	size_t _nBlockSize;
	//�½���
	std::mutex _mutex;
};

//����nSize nBlockSize������ڴ�Ĵ�С
template<size_t nSize, size_t nBlockSize> class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//ȷ��һ����4��������
		//8 4   61/8=7  61%8=5
		const size_t n = sizeof(void*);
		//(7*8)+8 
		_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		_nBlockSize = nBlockSize;
	}
};
//�ڴ������
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}

	~MemoryMgr()
	{

	}
public:
	static MemoryMgr& Instance()
	{
		//����ģʽ ��̬ ͨ��˽�л����캯�����㶨
		static MemoryMgr mgr;
		return mgr;
	}
	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}

	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)//���ڴ����
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else//�����ڴ����
		{
			if (--pBlock->nRef == 0)
				free(pBlock);//���ͷ�ļ�
		}
	}


	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}
private:
	MemoryAlloctor<64, 10> _mem64;
	MemoryAlloctor<128, 100000> _mem128;
	MemoryAlloctor<256, 100000> _mem256;
	MemoryAlloctor<512, 100000> _mem512;
	MemoryAlloctor<1024, 100000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];
};
#endif