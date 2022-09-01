#include"myclass.h"


_declspec(thread) ThreadCache* pTLSThreadCache = nullptr;
_declspec(thread) vector<void*>arr;

void fun()
{
	if (pTLSThreadCache == nullptr)
	{
		pTLSThreadCache = new ThreadCache();
	}
	for (int i = 0; i < 1030; i++)
	{
		arr.push_back(pTLSThreadCache->myMalloc(256));
		//cout<< this_thread::get_id() << ":" << arr[i] << endl;
	}
	for (int i = 0; i < 1030; i++)
	{
		pTLSThreadCache->myFree(arr[i], 256);
		//cout << this_thread::get_id() << "ÊÍ·Å:" << arr[i] << endl;
	}
}


int main()
{
	thread th1(fun);
	thread th2(fun);
	th2.join();
	th1.join();
	return 0;
}