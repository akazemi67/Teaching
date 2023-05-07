#include <windows.h>
#include <cstdio>

#define MAX_JOBS 64
HANDLE semaphoreHandles[MAX_JOBS];

VOID CALLBACK SimpleJob(_Inout_ PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID Context) {
	auto* const semHandle = Context;
	const auto currentThreadId = GetCurrentThreadId();
	ReleaseSemaphoreWhenCallbackReturns(Instance, semHandle, 1);

	for (auto i = 0; i < 60; i++) {
		printf("Running %d on thread with id: %lu\n",
			i, currentThreadId);
		Sleep(1000);
	}

	/*if(!ReleaseSemaphore(semHandle, 1, nullptr)) {
		printf("Error releasing semaphore by thread %lu error (%lu)\n",
			currentThreadId, GetLastError());
	}*/
}

int main() {
	int jobs;
	char createPool;

	printf("Create private thread pool(y/n): ");
	scanf_s("%c", &createPool);
	printf("Enter number of jobs: ");
	scanf_s("%d", &jobs);
	if (jobs > MAX_JOBS) {
		printf("Number of jobs should be less than 64.\n");
		exit(1);
	}

	PTP_CALLBACK_ENVIRON env = nullptr;
	PTP_POOL pool = nullptr;
	if (createPool == 'y') {
		pool = CreateThreadpool(nullptr);
		SetThreadpoolThreadMinimum(pool, 4);
		SetThreadpoolThreadMaximum(pool, 10);

		env = new TP_CALLBACK_ENVIRON;
		InitializeThreadpoolEnvironment(env);
		SetThreadpoolCallbackPool(env, pool);
	}

	for (auto i = 0; i < jobs; i++) {
		semaphoreHandles[i] = CreateSemaphore(
			nullptr,// default security attributes
			0,		// initial count
			1,		// maximum count
			nullptr// unnamed semaphore
		);
		if (semaphoreHandles[i] == nullptr) {
			printf("Error creating event %d error (%lu)\n",
				i, GetLastError());
			exit(1);
		}

		TrySubmitThreadpoolCallback(SimpleJob, semaphoreHandles[i], env);
	}

	WaitForMultipleObjects(jobs, semaphoreHandles,
		TRUE, INFINITE);

	if (env) {
		DestroyThreadpoolEnvironment(env);
		CloseThreadpool(pool);
		delete env;
	}
	for (auto i = 0; i < jobs; i++) {
		CloseHandle(semaphoreHandles[i]);
	}

	printf("---->>>>  Program finished  <<<<----\n");
	return 0;
}
