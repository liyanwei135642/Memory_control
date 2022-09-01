#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<map>
#include<string>
#include<vector>
#include<list>
#include<stack>
#include<queue>
#include<mutex>
#include<thread>
#include<atomic>
#include<algorithm>
#include<windows.h>
#include<memoryapi.h>
#include<assert.h>
#include<unordered_map>
#include<unordered_set>

using namespace std;

typedef unsigned long long PAGENUM;

/// <summary>
/// 单例模式 懒汉模式
/// </summary>
class SingleTemplate
{
	SingleTemplate();

	SingleTemplate(const SingleTemplate& st) = delete;

	SingleTemplate& operator=(const SingleTemplate& st) = delete;

	int num;

public:

	static SingleTemplate* getSingleTemplate();

	void print();
};


/// <summary>
/// 全排列
/// </summary>
/// <param name="t"></param>
/// <returns></returns>
vector<vector<int>> myAllPermutation(vector<int>& arr);

//全排列
vector<vector<int>> allPermutation(vector<int>& arr);

/// <summary>
/// 简单内存池
/// </summary>
/// <typeparam name="T"></typeparam>
template<class T>
class MyMemoryPool
{
	char* _memory = nullptr;//当前剩余内存地址
	T* _freeListNode = nullptr;//自由链表头指针
	size_t _remainSize = 0;//剩余空间数
	int _nodeSize;//分割单元空间大小
	vector<char*> arr;
public:

	//构造：确定分割单元空间大小至少大于等于指针的大小
	MyMemoryPool()
	{
		_nodeSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
	}

	//模拟构造T返回一个T类型指针
	T* myNew()
	{
		//返回值ret
		T* ret = nullptr;
		//自由链表不空的话先去自由链表找头部，头删！
		if (_freeListNode)
		{
			ret = _freeListNode;
			_freeListNode = (*(T**)_freeListNode);
			//用ret显式构造T类型
			new(ret)T;
			return ret;
		}
		if (_remainSize < _nodeSize)
		{
			//默认开128kb的空间
			_remainSize = 128 * 1024;
			//跳过malloc 直接用系统调用申请内存
			//_memory = (char*)VirtualAlloc(NULL, _remainSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			_memory = (char*)malloc(_remainSize);
			arr.push_back(_memory);
			if (_memory == nullptr)
			{
				cout << "开空间失败：";
				exit(-1);
			}
		}
		ret = (T*)_memory;
		//_memory向后走 _nodeSize 个大小
		_memory += _nodeSize;
		//剩余空间减小 _nodeSize 个大小
		_remainSize -= _nodeSize;
		//用ret显式构造T类型
		new(ret)T;
		return ret;
	}

	//模拟析构T释放掉一个不用的T类型
	void myDelete(T* p)
	{
		//头插
		*(T**)p = _freeListNode;
		_freeListNode = p;
	}

	//释放掉所有资源
	~MyMemoryPool()
	{
		for (auto& e : arr)
		{
			delete e;
		}
	}

};

/// <summary>
/// 计算FreeList的下标映射
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
int posCount(int size);

/// <summary>
/// 计算size 个字节对应的真实内存大小
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
int sizeCount(int size);

class FreeList
{
	void* head = nullptr;
	int Max = 1;
	size_t count=0;
public:

	//头插
	void push(void* p);

	//头删
	void* pop();

	//单次向中心申请的最大个数
	int& getMax();

	//头插多个节点
	void insert(void* start, void* end,size_t n);

	int multipop(void** start, void** end, int count);

	bool empty();

	size_t size();
};


struct Span
{
	unsigned long long start;
	size_t pageCount;
	FreeList freeList;
	size_t cap;
	size_t remain;
	Span* next=0;
	Span* prev=0;
	int multipop(void** start, void** end, int count);
	bool empty();
};

struct SpanList
{
	unordered_map<PAGENUM, Span*>map;
	Span* _head=0;
	std::mutex mux;
	SpanList()
	{
		_head = new Span();
		_head->next = _head;
		_head->prev = _head;
	}
	bool empty();
	void insert(Span* newSpan);
	Span* pop();
	Span* span();
	


};

struct Memory
{
	char* _c = 0;
	PAGENUM _pagenum = 128;
	unordered_set<Span*> arr;
	Memory(char* c=0):_c(c){}
};

class PageCache
{
	SpanList spanList[128];
	
	PageCache() {}
	PageCache(PageCache& pc) = delete;
	PageCache& operator=(PageCache& pc) = delete;
	static PageCache pCache;
public:

	unordered_map<PAGENUM, Memory*>map;

	std::mutex mux;

	static PageCache& getPageCache();

	//拿出一个Span
	Span* getSpan(int pageCount);

	//从中心缓存回收一个Span
	void  reclaim(Span* span);



};






class CentralCache
{
	SpanList spanList[208];
	CentralCache(){}
	CentralCache(CentralCache& cc) = delete;
	CentralCache& operator=(CentralCache& cc) = delete;
	static CentralCache cenChe;
public:

	static CentralCache& getCentralCache();

	///中心给线程缓存count个，返回实际给的个数
	size_t giveThread(void** start, void** end, int count,int size);

	//中心从线程缓存回收放回index位置
	void reclaim(void* start, void* end,int n, int index);




};



class ThreadCache
{
	FreeList freeList[208];
public:

	void* myMalloc(size_t size);

	void myFree(void* p, size_t size);

	void getMemoryFromCentral(size_t size);



};










