#include <Windows.h>
#include <cstdio>
#include <thread>

/**
 * Useful Links:
 * https://learn.microsoft.com/en-us/windows/win32/procthread/thread-local-storage
 * https://bsodtutorials.wordpress.com/2014/02/07/thread-local-storage-slots/
 * https://doxygen.reactos.org/de/de0/dll_2win32_2kernel32_2client_2except_8c.html#a58b552917f96de1c95ea05b0225ab2e5
 */
__declspec(thread) int value;//TLS

void TlsThreadFunc() {
	//Changing TLS variable
	printf("\n* Value inside Thread: 0x%x\n", value);
	value = 0x55aaaa55;
	printf("* Changed value inside Thread: 0x%x\n", value);

	//Doing something that causes an error
	__try {
		int ret = CloseHandle((HANDLE)22222);
		if (ret == 0) {
			printf("\n - Error in Close Handle: 0x%lx\n", 
				GetLastError());
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		printf(" - Exception in Close Handle with code: 0x%lx\n", 
			GetExceptionCode());
	}
}

void LoopingThreadFunc() {
	int i = 1;
	while(true) {
		Sleep(500);
		printf("  - Step %d\n", i++);
	}
}

void UnexecutedFunction() {
	printf("\n\n---------********---------\n");
	printf("This code should never be executed!\n");
	printf("---------********---------");
	ExitThread(0);
}

int main() {
	value = 0xdeadbeef;
	printf("Value before Thread: 0x%x\n", value);

	std::thread t(TlsThreadFunc);
	t.join();
	
	printf("\nValue after Thread: 0x%x\n", value);
	printf(" \n--> Press any key to continue....\n");
	getchar();
	
	//Accessing and examining thread context
	std::thread t2(LoopingThreadFunc);

	Sleep(5000);
	SuspendThread(t2.native_handle());
	
	CONTEXT ctx = { 0 };
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(t2.native_handle(), &ctx);

	//Changing thread execution path
	ctx.Rip = (DWORD_PTR)&UnexecutedFunction;
	SetThreadContext(t2.native_handle(), &ctx);
	
	ResumeThread(t2.native_handle());
	
	t2.join();

	printf("\n\n*** Program finished ***\n");
	return 0;
}

