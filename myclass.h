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
/// ����ģʽ ����ģʽ
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
/// ȫ����
/// </summary>
/// <param name="t"></param>
/// <returns></returns>
vector<vector<int>> myAllPermutation(vector<int>& arr);

//ȫ����
vector<vector<int>> allPermutation(vector<int>& arr);

/// <summary>
/// ���ڴ��
/// </summary>
/// <typeparam name="T"></typeparam>
template<class T>
class MyMemoryPool
{
	char* _memory = nullptr;//��ǰʣ���ڴ��ַ
	T* _freeListNode = nullptr;//��������ͷָ��
	size_t _remainSize = 0;//ʣ��ռ���
	int _nodeSize;//�ָԪ�ռ��С
	vector<char*> arr;
public:

	//���죺ȷ���ָԪ�ռ��С���ٴ��ڵ���ָ��Ĵ�С
	MyMemoryPool()
	{
		_nodeSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
	}

	//ģ�⹹��T����һ��T����ָ��
	T* myNew()
	{
		//����ֵret
		T* ret = nullptr;
		//���������յĻ���ȥ����������ͷ����ͷɾ��
		if (_freeListNode)
		{
			ret = _freeListNode;
			_freeListNode = (*(T**)_freeListNode);
			//��ret��ʽ����T����
			new(ret)T;
			return ret;
		}
		if (_remainSize < _nodeSize)
		{
			//Ĭ�Ͽ�128kb�Ŀռ�
			_remainSize = 128 * 1024;
			//����malloc ֱ����ϵͳ���������ڴ�
			//_memory = (char*)VirtualAlloc(NULL, _remainSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			_memory = (char*)malloc(_remainSize);
			arr.push_back(_memory);
			if (_memory == nullptr)
			{
				cout << "���ռ�ʧ�ܣ�";
				exit(-1);
			}
		}
		ret = (T*)_memory;
		//_memory����� _nodeSize ����С
		_memory += _nodeSize;
		//ʣ��ռ��С _nodeSize ����С
		_remainSize -= _nodeSize;
		//��ret��ʽ����T����
		new(ret)T;
		return ret;
	}

	//ģ������T�ͷŵ�һ�����õ�T����
	void myDelete(T* p)
	{
		//ͷ��
		*(T**)p = _freeListNode;
		_freeListNode = p;
	}

	//�ͷŵ�������Դ
	~MyMemoryPool()
	{
		for (auto& e : arr)
		{
			delete e;
		}
	}

};

/// <summary>
/// ����FreeList���±�ӳ��
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
int posCount(int size);

/// <summary>
/// ����size ���ֽڶ�Ӧ����ʵ�ڴ��С
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

	//ͷ��
	void push(void* p);

	//ͷɾ
	void* pop();

	//���������������������
	int& getMax();

	//ͷ�����ڵ�
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

	//�ó�һ��Span
	Span* getSpan(int pageCount);

	//�����Ļ������һ��Span
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

	///���ĸ��̻߳���count��������ʵ�ʸ��ĸ���
	size_t giveThread(void** start, void** end, int count,int size);

	//���Ĵ��̻߳�����շŻ�indexλ��
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










