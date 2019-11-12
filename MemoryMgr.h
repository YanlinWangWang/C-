#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>

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
		_nSzie = 0;
		_nBlockSzie = 0;
	}

	~MemoryAlloc()
	{

	}


	//申请内存
	//申请nSize大小的内存
	//一般进入这个就是用完了对应内存池空间
	void* allocMem(size_t nSize)
	{
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
			pReturn->pAlloc = this;
			pReturn->pNext = nullptr;
		}
		//要是内存池里面还有那么将头指向下一个节点
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	void freeMem(void* pMem)
	{
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
			free(pMem);
		}

	}

	//初始化
	void initMemory()
	{	//断言
		assert(nullptr == _pBuf);
		if (!_pBuf)
			return;
		//计算内存池的大小
		size_t bufSize = _nSzie * _nBlockSzie;
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
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小
	size_t _nSzie;
	//内存单元的数量
	size_t _nBlockSzie;
};
#endif