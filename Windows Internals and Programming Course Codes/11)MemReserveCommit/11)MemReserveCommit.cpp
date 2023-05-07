#include <Windows.h>
#include <cstdio>

int main() {
	printf("Press any key to commit 1GB memory.\n");
	getchar();

	void* addr = VirtualAlloc(nullptr, 1 << 30,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READWRITE);
	if(!addr) {
		printf("Error creating memory: %lx", GetLastError());
		return 1;
	}
	
	printf("Press any key to write 200MB values to the memory.\n");
	getchar();
	memset(addr, 0xA, 200 * (1 << 20));

	printf("Press any key to reserve 120TBs of memory.\n");
	getchar();
	
	void* p = VirtualAlloc(nullptr, 120LL * (1LL << 40),
		MEM_RESERVE,
		PAGE_READWRITE);
	if(!p) {
		printf("Error reserving terabytes of memory: %lx", GetLastError());
		exit(1);
	}
	
	printf("Press any key to exit...\n");
	getchar();
	return 0;
}
