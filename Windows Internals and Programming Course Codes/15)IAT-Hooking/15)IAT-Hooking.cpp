#include <Windows.h>
#include <cstdio>

int WINAPI hookedBeep(DWORD dwFreq, DWORD dwDuration) {
	typedef int(WINAPI* MessageBoxWType)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption,
		UINT uType);
	HMODULE hUser32 = LoadLibraryA("User32.dll");
	MessageBoxWType pMessageBoxW = (MessageBoxWType)GetProcAddress(hUser32, 
		"MessageBoxW");
	pMessageBoxW(nullptr, L"Showing MessageBox instead of beeping!", L"BeepBeep", MB_OK);
	return 0;
}

void ChangeIAT_HookBeep() {
	LPVOID imageBase = GetModuleHandleA(nullptr);
	PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)imageBase;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)
					((DWORD_PTR)imageBase + dosHeaders->e_lfanew);

	IMAGE_DATA_DIRECTORY importsDirectory = 
		ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = 
		(PIMAGE_IMPORT_DESCRIPTOR)(importsDirectory.VirtualAddress + (DWORD_PTR)imageBase);
	
	while (importDescriptor->Name) {
		LPCSTR libraryName = (LPCSTR)importDescriptor->Name + (DWORD_PTR)imageBase;
		HMODULE library = LoadLibraryA(libraryName);
		
		if (library) {
			PIMAGE_THUNK_DATA originalFirstThunk_INT = nullptr;	//RVA of Import Name Table 
			PIMAGE_THUNK_DATA firstThunk_IAT = nullptr;			//RVA of Import Address Table
			
			originalFirstThunk_INT = (PIMAGE_THUNK_DATA)
				((DWORD_PTR)imageBase + importDescriptor->OriginalFirstThunk);
			firstThunk_IAT = (PIMAGE_THUNK_DATA)
				((DWORD_PTR)imageBase + importDescriptor->FirstThunk);
			while (originalFirstThunk_INT->u1.AddressOfData) {
				PIMAGE_IMPORT_BY_NAME functionName = (PIMAGE_IMPORT_BY_NAME)
					((DWORD_PTR)imageBase + originalFirstThunk_INT->u1.AddressOfData);
				
				if (strcmp(functionName->Name, "Beep") == 0) {
					DWORD oldProtect = 0;
					VirtualProtect((LPVOID)(&firstThunk_IAT->u1.Function),
						8, PAGE_READWRITE, &oldProtect);
					firstThunk_IAT->u1.Function = (DWORD_PTR)hookedBeep;
				}
				
				originalFirstThunk_INT++;
				firstThunk_IAT++;
			}
		}
		importDescriptor++;
	}
}

int main() {
	for (int i = 1500; i < 3000; i += 10) {
		Beep(i, 100);
	}

	printf("Press any key to hook Beep!\n");
	getchar();

	ChangeIAT_HookBeep();
	Beep(0, 0);
	
	return 0;
}
