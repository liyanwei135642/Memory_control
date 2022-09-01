#include "myclass.h"

SingleTemplate* SingleTemplate:: getSingleTemplate()
{
	static mutex mylock;
	static SingleTemplate* ptr;
	if (ptr)
		return ptr;
	mylock.lock();
	if (ptr == NULL)
		ptr = new SingleTemplate();
	mylock.unlock();
	return ptr;
}

SingleTemplate::SingleTemplate():num(88){}

void SingleTemplate:: print()
{
	cout << num;
}


/// <summary>
/// 全排列
/// </summary>
/// <param name="t"></param>
/// <returns></returns>
vector<vector<int>> myAllPermutation(vector<int>& arr)
{
	vector<vector<int>>ret;
	int n = arr.size();
	if (n < 2)
	{
		ret.push_back(arr);
		return ret;
	}
	int key = arr.back();
	arr.pop_back();
	vector<vector<int>>arr1 = myAllPermutation(arr);
	for (auto& e : arr1)
	{
		for (int i = 0; i < n; i++)
		{
			vector<int> arr2(n);
			for (int j = 0; j < n; j++)
			{
				if (j < i)
					arr2[j] = e[j];
				else if (j == i)
					arr2[j] = key;
				else
					arr2[j] = e[j - 1];
			}
			ret.push_back(arr2);
		}
	}
	return ret;
}


vector<vector<int>> allPermutation(vector<int>& arr)
{
	vector<vector<int>>ret;
	sort(arr.begin(), arr.end());
	ret.push_back(arr);
	while (next_permutation(arr.begin(), arr.end()))
	{
		ret.push_back(arr);
	}
	return ret;
}


/// <summary>
/// 计算FreeList的下标映射
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
int posCount(int size)
{
	if (size <= 128)
	{
		return (size - 1) / 8;
	}
	else if (size <= 1024)
	{
		return 16 + (size - 129) / 16;
	}
	else if (size <= 8 * 1024)
	{
		return 72 + (size - 1025) / 128;
	}
	else if (size <= 64 * 1024)
	{
		return 128 + (size - 8 * 1024 - 1) / 1024;
	}
	else
	{
		return 184 + (size - 64 * 1024 - 1) / (8 *1024);
	}
}


/// <summary>
/// 计算size 个字节对应的真实内存大小
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
int sizeCount(int size)
{
	if (size <= 128)
	{
		return ((size - 1) / 8+1)*8;
	}
	else if (size <= 1024)
	{
		return ((size - 1) / 16+1)*16;
	}
	else if (size <= 8 * 1024)
	{
		return ((size - 1) / 128+1)*128;
	}
	else if (size <= 64 * 1024)
	{
		return ((size - 1) / 1024+1)*1024;
	}
	else
	{
		return ((size - 1) / (8 * 1024)+1)*8*1024;
	}
}

/// <summary>
/// 计算一次向中心申请的个数
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
int minSize(size_t size)
{
	int ret = 256 * 1024 / size;
	if (ret < 2)
		return 2;
	else if (ret > 526)
		return 526;
	else
		return ret;
}

//头插
void FreeList::push(void* p)
{
	assert(p);
	*(void**)p = head;
	head = p;
	count++;
}

//头删
void* FreeList:: pop()
{
	if (head == nullptr)
		return head;
	void* ret = head;
	head = *(void**)head;
	count--;
	return ret;
}

int& FreeList::getMax()
{
	return Max;
}

//头插多个节点
void FreeList::insert(void* start, void* end,size_t n)
{
	*(void**)end = head;
	head = start;
	count += n;
}

int FreeList::multipop(void** start, void** end, int n)
{
	assert(head);
	*start = head;
	int i = 0;
	for (i = 1; i < n; i++)
	{
		if (*(void**)head == nullptr)
		{
			break;
		}
		head = *(void**)head;
	}
	*end = head;
	head = *(void**)head;
	count -= i;
	return i;
}

bool FreeList::empty()
{
	return head == nullptr;
}

size_t FreeList:: size()
{
	return count;
}

PageCache PageCache::pCache;

PageCache& PageCache::getPageCache()
{
	return pCache;
}


//拿出一个Span
Span* PageCache:: getSpan(int pageCount)
{
	int n = pageCount - 1;
	int i = 0;
	for (i = n; i < 128; i++)
	{
		if (!spanList[i].empty())
			break;
	}
	if (i == n)
	{
		Span*span=spanList[i].pop();
		map[span->start]->_pagenum -= span->pageCount;
		map[span->start]->arr.erase(span);
		return span;
	}
	if (i < 128)
	{
		Span* span1 = spanList[i].pop();
		map[span1->start]->arr.erase(span1);
		Span* span2 = new Span;
		span2->start = span1->start + pageCount;
		span2->pageCount = span1->pageCount - pageCount;
		span1->pageCount = pageCount;
		spanList[i - n - 1].insert(span2);
		map[span2->start]->arr.insert(span2);
		map[span1->start]->_pagenum -= span1->pageCount;
		return span1;
	}
	void* ptr = (char*)VirtualAlloc(NULL, 128 * 8 * 1024, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Span* span = new Span;
	span->start = (unsigned long long)ptr >> 13;
	span->pageCount = 128;
	spanList[127].insert(span);
	Memory* m = new Memory((char*)ptr);
	for (PAGENUM i = span->start; i < span->start + span->pageCount; i++)
	{
		map[i]=m;
	}
	m->arr.insert(span);
	return getSpan(pageCount);
}

//从中心缓存回收一个Span
void PageCache:: reclaim(Span* span)
{
	mux.lock();
	int n = span->pageCount - 1;
	spanList[n].insert(span);
	PAGENUM id = span->start;
	Memory* m = map[id];
	m->_pagenum += span->pageCount;
	m->arr.insert(span);
	if (m->_pagenum == 128)
	{
		for (auto& e : m->arr)
		{
			e->prev->next = e->next;
			e->next->prev = e->prev;
		}
		VirtualFree(map[id]->_c, 1024 * 1024, MEM_RELEASE);
		PAGENUM s = ((PAGENUM)m->_c)>>13;
		for (PAGENUM i = s; i < s + 128; i++)
		{
			map.erase(i);
		}
		delete m;
	}
	mux.unlock();
}

CentralCache CentralCache::cenChe;

int Span:: multipop(void** start, void** end, int count)
{
	int ret = 0;
	ret = freeList.multipop(start, end, count);
	remain -= ret;
	return ret;
}

bool Span::empty()
{
	return freeList.empty();
}

bool SpanList:: empty()
{
	return _head->next == _head;
}

Span* SpanList:: span()
{
	Span* ret = _head->next;
	while (ret != _head)
	{
		if (!ret->empty())
		{
			return ret;
		}
		ret = ret->next;
	}
	return nullptr;
}

void SpanList:: insert(Span* newSpan)
{
	newSpan->prev = _head;
	newSpan->next = _head->next;
	_head->next->prev = newSpan;
	_head->next = newSpan;
	for (PAGENUM i = newSpan->start; i < newSpan->start + newSpan->pageCount;i++)
	{
		map[i] = newSpan;
	}
}

Span* SpanList:: pop()
{
	Span* ret = _head->next;
	_head->next = ret->next;
	ret->next->prev = _head;
	for (PAGENUM i = ret->start; i < ret->start + ret->pageCount; i++)
	{
		map.erase(i);
	}
	return ret;
}

//中心给线程缓存count个，返回实际给的个数
size_t CentralCache:: giveThread(void** start, void** end, int count,int size)
{
	size_t ret = 1;
	int index = posCount(size);
	SpanList& spanlist = spanList[index];
	spanlist.mux.lock();
	Span* span = spanlist.span();
	if (span==nullptr)
	{
		int bytesize = sizeCount(size);
		int n=((count* bytesize )>> 13)+1;
		PageCache::getPageCache().mux.lock();
		span=PageCache::getPageCache().getSpan(n);
		PageCache::getPageCache().mux.unlock();
		size_t N= n * 1024 * 8 / bytesize;
		span->cap = N;
		span->remain = N;
		void* begin = (void*)(span->start * 8 * 1024);
		void* ends = begin;
		for (int i = 1; i < N; i++)
		{
			void*tem= (void*)((char*)ends + bytesize);
			*(void**)ends = tem;
			ends = tem;
		}
		span->freeList.insert(begin, ends,N);
		spanlist.insert(span);
	}
	ret=span->multipop(start, end, count);
	spanlist.mux.unlock();
	return ret;
}

//中心从线程缓存回收n个放回index位置
void CentralCache:: reclaim(void* start, void* end,int n,int index)
{
	SpanList& spanlist = spanList[index];
	FreeList list;
	list.insert(start, end, n);
	spanlist.mux.lock();
	while (!list.empty())
	{
		void* p = list.pop();
		unsigned long long start = ((unsigned long long)p) >> 13;
		Span* next = spanlist.map[start];
		next->freeList.push(p);
		next->remain++;
		//cout << "central reclaim " << p << "->" << next->start<<endl;
		if (next->remain < next->cap)
			continue;
		next->prev->next = next->next;
		next->next->prev = next->prev;
		PageCache::getPageCache().reclaim(next);
		//cout << "page reclaim " << next->start <<endl;
		assert(next != spanlist._head);
	}
	spanlist.mux.unlock();
}


CentralCache& CentralCache::getCentralCache()
{
	return cenChe;
}


void* ThreadCache:: myMalloc(size_t size)
{
	int n = posCount(size);
	void* ret = freeList[n].pop();
	if (ret)
		return ret;
	getMemoryFromCentral(size);
	ret=freeList[n].pop();
	return ret;
}

void  ThreadCache::myFree(void* p, size_t size)
{
	int N = posCount(size);
	FreeList& list = freeList[N];
	list.push(p);
	size_t n = list.getMax();
	if (n >= list.size())
		return;
	void* start = 0;
	void* end = 0;
	list.multipop(&start, &end, n);
	CentralCache::getCentralCache().reclaim(start, end,n,N);
}

void  ThreadCache::getMemoryFromCentral(size_t size)
{
	int index = posCount(size);
	int byteSize = sizeCount(size);
	int n1 = minSize(byteSize);
	int& n2 = freeList[index].getMax();
	int n = 0;
	if (n2 < n1)
	{
		n = n2;
		n2++;
	}
	else
	{
		n = n1;
	}
	void* start=0;
	void* end=0;
	size_t m=CentralCache::getCentralCache().giveThread(&start, &end, n,size);
	freeList[index].insert(start, end,m);
}


