#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>
#include <mutex>

//默认大小
#define MAX_MEMORY_SZIE 1024

#ifdef _DEBUG
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif // _DEBUG

class MemoryAlloc;
//内存块 最小单元
class MemoryBlock
{
public:
	//所属大内存块（池）
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//是否在内存池中
	bool bPool;
private:
	//预留
	char c1;
	char c2;
	char c3;
};

//分配这块内存
class MemoryAlloc
{
public:
	//初始化函数
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


	//申请内存
	//申请nSize大小的内存
	//一般进入这个就是用完了对应内存池空间
	void* allocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		//没有初始化那么就初始化内存分配
		if (!_pBuf)
		{
			initMemory();
		}

		//pReturn返回指针
		MemoryBlock* pReturn = nullptr;
		//判断内存池用完了没有
		if (nullptr == _pHeader)
		{
			//申请这么大的空间 并且还要申请头节点
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 0;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		//要是内存池里面还有那么将头指向下一个节点
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		//输出打印
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		//首先判断是内存池还是自主申请的内存
		//求内存块的块头
		/*
		地址结构如下
		内存块头文件      可以使用的地址
		                  ^1
						  这个地方是传入的地址也就是pMem
		所以要减去(MemoryBlock)大小的位置 让他指向头节点
		*/
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));

		//因为每个节点都只会被使用一次
		assert(pBlock->nRef == 1);

		//如果引用数不为1那么为0
		if (--pBlock->nRef != 0)
		{
			return;
		}

		//看看是从内存池中申请到的内存还是直接从OS当中申请的内存
		if (pBlock->bPool)//在内存池当中
		{
			//那么将它直接插入到内存空闲链表的第一个
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		//不在那么直接释放
		else
		{
			free(pBlock);
		}

	}

	//初始化
	void initMemory()
	{	//断言
		assert(nullptr == _pBuf);
		if (_pBuf)//空就直接返回
			return;
		//计算内存池的大小
		size_t realSzie = _nSize + sizeof(MemoryBlock);//真正的大小 
		size_t bufSize = realSzie * _nBlockSize;
		//向系统申请池的内存
		_pBuf = (char*)malloc(bufSize);

		//初始化内存池
		//首先初始化第一块内存 （也就是内存池链表的链表头）
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;

		//遍历内存块进行初始化
		//然后遍历剩下的n块内存进行遍历
		//依次初始化对应节点 并且将节点加入到对应的内存链表队列中去
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
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小
	size_t _nSize;
	//内存单元的数量
	size_t _nBlockSize;
	//新建锁
	std::mutex _mutex;
};

//其中nSize nBlockSize是这个内存的大小
template<size_t nSize, size_t nBlockSize> class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//确保一定是4的整数倍
		//8 4   61/8=7  61%8=5
		const size_t n = sizeof(void*);
		//(7*8)+8 
		_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		_nBlockSize = nBlockSize;
	}
};
//内存管理工具
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
		//单例模式 静态 通过私有化构造函数来搞定
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
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

	//释放内存
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)//在内存池中
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else//不在内存池中
		{
			if (--pBlock->nRef == 0)
				free(pBlock);//清除头文件
		}
	}


	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//初始化内存池映射数组
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