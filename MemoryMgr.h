#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>

//Ĭ�ϴ�С
#define MAX_MEMORY_SZIE 64

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
		if (_pBuf)
			free(_pBuf);
	}


	//�����ڴ�
	//����nSize��С���ڴ�
	//һ�����������������˶�Ӧ�ڴ�ؿռ�
	void* allocMemory(size_t nSize)
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
	void freeMemory(void* pMem)
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
			free(pBlock);
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

template<size_t nSize, size_t nBlockSize> class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//ȷ��һ����4��������
		//8 4   61/8=7  61%8=5
		const size_t n = sizeof(void*);
		//(7*8)+8 
		_nSzie = (nSzie / n) * n + (nSzie % n ? n : 0);
		_nBlockSize = nBlockSize;
	}
};
//�ڴ������
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init(0, 64, &_mem64);
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
			return pReturn;
		}
	}

	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		if (pBlock->bPool)//���ڴ����
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else//�����ڴ����
		{
			if (--pBlock->nRef == 0)
				free(pMem);
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
	void init(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}
private:
	MemoryAlloctor<64, 10> _mem64;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];
};
#endif