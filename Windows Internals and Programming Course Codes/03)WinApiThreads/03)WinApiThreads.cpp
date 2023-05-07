#include <Windows.h>
#include <cstdio>
#include <ctime>

#define MAX_THREADS 3

HANDLE sem;
int value;

DWORD WINAPI IncrementalThreads(PVOID param) {
	auto tid = *static_cast<int*>(param);

	//Sleeps are for simulating a task that takes time.
	for (auto x = 0; x < 10; x++) {
		for (auto step = 0; step < MAX_THREADS; step++) {
			//Never use Sleep in a CriticalSection
			WaitForSingleObject(sem, INFINITE);
			auto temp = value;
			if (tid == step) {
				Sleep(rand() % 50);
			}
			value = temp + 1;
			ReleaseSemaphore(sem, 1, nullptr);
		}
	}
	return 0;
}

int main() {
	HANDLE threads[MAX_THREADS];
	int thread_inputs[MAX_THREADS];

	sem = CreateSemaphore(
		nullptr,// default security attributes
		1,		// initial count
		1,		// maximum count
		nullptr// unnamed semaphore
	);
	
	
	srand(time(NULL));
	printf("- Value before threads: %d\n", value);
	
	for(auto i=0; i<MAX_THREADS; ++i ) {
		thread_inputs[i] = i;
		threads[i] = CreateThread(
			nullptr, //1) security descriptor for thread. NULL=default
			0,  /*2) Initial stack size in bytes.
						   The system rounds this value to the nearest page.
						   zero == default size for the executable*/

			& IncrementalThreads,	//3) Thread Function
			&thread_inputs[i],			//4) Thread function parameter

			0, /*5) 0 == The thread runs immediately.
						   CREATE_SUSPENDED:
							 The thread is created in a suspended state,
							 and does not run until the ResumeThread is called.*/

			nullptr //6) A pointer to a variable to get thread identifier.
		);
	}

	WaitForMultipleObjects(MAX_THREADS, threads, 
		true, INFINITE);
	printf("- Value AFTER threads: %d\n", value);

	for (auto& handle : threads) {
		CloseHandle(handle);
	}
	CloseHandle(sem);
	
	return 0;
}


