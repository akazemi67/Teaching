#include <Windows.h>
#include <cstdio>

class IUseMyOwnHeap {
private:
	static HANDLE hHeap;
	static int nObjCount;
	//class data should be here!
	int a, b;
public:
	IUseMyOwnHeap(int _a, int _b): a(_a), b(_b){}

	int doSum() {
		return a + b;
	}
	
	void* operator new(size_t size){
		if (nObjCount++ == 0)
			hHeap = HeapCreate(0, 1 << 20, 10 * (1 << 20));
		return HeapAlloc(hHeap, 0, sizeof(IUseMyOwnHeap));
	}

	void operator delete(void *p){
		HeapFree(hHeap, 0, p);
		if(--nObjCount == 0) {
			HeapDestroy(hHeap);
		}
	}
};
HANDLE IUseMyOwnHeap::hHeap = nullptr;
int IUseMyOwnHeap::nObjCount = 0;

int main() {
	printf("Press any key to create heap.\n");
	getchar();
	IUseMyOwnHeap* obj = new IUseMyOwnHeap(12, 44);

	printf("Press any key to create 50 objects.\n");
	getchar();
	IUseMyOwnHeap* arr[50];
	for(int i=0; i<50; i++) {
		arr[i] = new IUseMyOwnHeap(10 * i, 20 * i);
	}

	printf("Press any key to delete 50 objects.\n");
	getchar();
	for (int i = 0; i < 50; i++)
		delete arr[i];
	
	printf("Press any key to delete heap.\n");
	getchar();
	delete obj;

	printf("Press any key to exit program.\n");
	getchar();
	return 0;
}
