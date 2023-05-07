#include <windows.h>
#include <cstdio>

#define MAX_WORKS 1000
PTP_WORK CreatedWorks[MAX_WORKS];

VOID CALLBACK WorkFunction(_Inout_ PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID Context,
	_Inout_ PTP_WORK Work) {

	const auto currentThreadId = GetCurrentThreadId();
	for (auto i = 0; i < 10; i++) {
		printf("Running %d on thread with id: %lu\n",
			i, currentThreadId);
		Sleep(1000);
	}
}

int main() {
	int jobs, i;
	printf("Enter number of jobs: ");
	scanf_s("%d", &jobs);

	for(i=0; i<jobs; i++) {
		CreatedWorks[i] = CreateThreadpoolWork(WorkFunction,nullptr, 
			nullptr);
		SubmitThreadpoolWork(CreatedWorks[i]);
	}

	for(i=0; i<jobs; i++) {
		WaitForThreadpoolWorkCallbacks(CreatedWorks[i], 
			FALSE);
	}

	for(i=0; i<jobs; i++) {
		CloseThreadpoolWork(CreatedWorks[i]);
	}
	printf("---->>>>  Program finished  <<<<----\n");
	return 0;
}
