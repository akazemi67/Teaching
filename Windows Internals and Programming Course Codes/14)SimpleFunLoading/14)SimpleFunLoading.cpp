#include <Windows.h>
#include <winnt.h>
#include <winternl.h>
#include <cstdio>

void ChangePeb() {
	PROCESS_BASIC_INFORMATION ProcessInformation;
	ULONG length;
	NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation,
		&ProcessInformation, sizeof(ProcessInformation), &length);

	UNICODE_STRING notepad;
	RtlInitUnicodeString(&notepad, L"C:\\windows\\system32\\notepad.exe");
	ProcessInformation.PebBaseAddress->ProcessParameters->CommandLine = notepad;
	ProcessInformation.PebBaseAddress->ProcessParameters->ImagePathName = notepad;
}

/*int sum(int a, int b) {
	return a + b;
}*/

int main() {
	printf("Hello!\n");
	
	typedef int(WINAPI* MessageBoxWType)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, 
		UINT uType);
	HMODULE hUser32 = LoadLibraryA("User32.dll");
	MessageBoxWType pMessageBoxW = (MessageBoxWType)GetProcAddress(hUser32, 
		"MessageBoxW");
	pMessageBoxW(nullptr, L"Hello from Message Box!", L"hi", MB_OK);

	//int(*f)(int a, int b) = &sum;
	//printf("%d",f(1, 2));
	
	printf("Press any key to change PEB!\n");
	getchar();
	ChangePeb();

	printf("Press any key to exit!\n");
	getchar();
	return 0;
}
