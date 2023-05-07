#include <Windows.h>
#include <cstdio>

struct MyThreadInput {
	int from, to;
};

DWORD WINAPI MyThreadFunction(PVOID param) {
	auto* inp = static_cast<MyThreadInput*>(param);
	for (auto i = inp->from; i < inp->to; i++) {
		printf("Thread %lu is on: %d\n", GetCurrentThreadId(), i);
		Sleep(500);
	}

	return 0;
}

int main() {
	HANDLE handles[5];
	MyThreadInput inputs[5];

	for (auto i = 0; i < 5; i++) {
		inputs[i].from = i * 1000;
		inputs[i].to = (i + 1) * 1000;

		handles[i] = CreateThread(
			nullptr, //1) security descriptor for thread. NULL=default
			0,  /*2) Initial stack size in bytes.
						   The system rounds this value to the nearest page.
						   zero == default size for the executable*/

			&MyThreadFunction,	//3) Thread Function
			&inputs[i],			//4) Thread function parameter

			0, /*5) 0 == The thread runs immediately.
						   CREATE_SUSPENDED:
							 The thread is created in a suspended state,
							 and does not run until the ResumeThread is called.*/

			nullptr //6) A pointer to a variable to get thread identifier.
		);
	}

	WaitForMultipleObjects(5, handles, 
		true, INFINITE);

	for (auto& handle : handles) {
		CloseHandle(handle);
	}
	
	printf("\nMain is going to return...\n\n");
	return 0;
}

