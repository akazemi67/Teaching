#include <Windows.h>
#include <winternl.h>

extern "C" {
    NTSTATUS NTAPI NtTerminateProcess(_In_opt_ HANDLE ProcessHandle, 
        _In_ NTSTATUS ExitStatus);
	//Sleep
    NTSTATUS NTAPI NtDelayExecution(_In_ BOOLEAN Alertable, 
        _In_opt_ PLARGE_INTEGER DelayInterval);
	//printf
    NTSTATUS NTAPI NtDrawText(_In_ PUNICODE_STRING Text);
}

#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)

//Process Hacker
extern "C" void NTAPI NtProcessStartup(PPEB peb) {
    PROCESS_BASIC_INFORMATION info;
    NtQueryInformationProcess(NtCurrentProcess(), 
        ProcessBasicInformation,
        &info, sizeof(info), 
        nullptr);
    UNICODE_STRING text;
    RtlInitUnicodeString(&text, L"Hello Memory Analysis from Native World!!!");
    NtDrawText(&text);

	/*
	Specifies the absolute or relative time, in units of 100 nanoseconds,
	for which the wait is to occur. A negative value indicates relative time.

	Note for absolute values:
	Contains a 64-bit value representing the number of
	100-nanosecond intervals since January 1, 1601 (UTC).
	 */
    LARGE_INTEGER interval;
    interval.QuadPart = -10000 * 10000;
    NtDelayExecution(FALSE, &interval);
    NtTerminateProcess(NtCurrentProcess(), 0);
}
