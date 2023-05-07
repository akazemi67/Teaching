#include <Windows.h>
#include <cstdio>

int main(int argc, char **argv) {
	LPCSTR dllPath = "D:\\WindowsInternalsSamples\\x64\\Debug\\12)SimpleDll.dll";
	if(argc<2) {
		printf("Usage: injector.exe <pid>\n");
		exit(0);
	}
	
	const int processId = atoi(argv[1]);
	const int bufferLen = strlen(dllPath) + 1;

	//1) Attach to the process
	HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS,
		FALSE,
		processId);
	if(procHandle==INVALID_HANDLE_VALUE) {
		printf("Error openning handle to process %d code: %lx\n",
			processId, GetLastError());
		exit(1);
	}
	
	//2) Buffer creation
	LPVOID remoteBufferAddr = VirtualAllocEx(procHandle,
		nullptr,
		bufferLen,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
	if(!remoteBufferAddr) {
		printf("Error allocating buffer in the process %d code: %lx\n",
			processId, GetLastError());
		exit(1);
	}

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

	//TODO cleanup
	return 0;
}
