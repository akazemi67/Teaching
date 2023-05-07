#include <windows.h>
#include <cstdio>

VOID CALLBACK TimerFunction(_Inout_ PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID Context,
	_Inout_ PTP_TIMER Timer) {
	static int step = 1;
	printf("Running %d on thread with id: %lu\n",
		step, GetCurrentThreadId());
	step++;
}

int main() {
	auto* Timer = CreateThreadpoolTimer(TimerFunction, nullptr,
	                                    nullptr);
	if(!Timer) {
		printf("Error creating timer. Error(%lu)\n",
			GetLastError());
		return 1;
	}

	LONG64 dueTime = -10000 * 1000;
	SetThreadpoolTimer(Timer,
		reinterpret_cast<FILETIME*>(&dueTime), /*specifies 
                 the absolute or relative time
				 at which the timer should expire*/
		1000,//period, in milliseconds

		0/*The maximum amount of time the system can delay 
		 before calling the timer callback.*/
	);

	Sleep(10000);
	WaitForThreadpoolTimerCallbacks(Timer, TRUE);
	CloseThreadpoolTimer(Timer);
	
	return 0;
}
