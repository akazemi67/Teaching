#include <cstdlib>
#include "phnt_windows.h"
#include "phnt.h"
#include "ntpebteb.h"

typedef unsigned long NULONG;

#ifdef _M_IX86 
static __inline PEB __declspec(naked) __forceinline* GetPEBx86() {
	__asm {
		mov eax, dword ptr fs : [0x30] ;
		retn;
	}
}
#endif

HMODULE WINAPI GetModuleBaseAddress(LPCWSTR moduleName) {
	_PEB* pPeb = nullptr;
#ifdef _M_IX86 
	pPeb = GetPEBx86();
#elif defined(_M_AMD64)
	pPeb = (_PEB*)__readgsqword(0x60);
#endif

	if (!pPeb)
		return nullptr;

	LIST_ENTRY* pListHead = &pPeb->Ldr->InLoadOrderModuleList;
	LDR_DATA_TABLE_ENTRY* pLdrDataTableEntry = (PLDR_DATA_TABLE_ENTRY)pListHead->Flink;
	do {
		if (!lstrcmpiW(pLdrDataTableEntry->BaseDllName.Buffer, moduleName))
			return (HMODULE)pLdrDataTableEntry->DllBase;

		pLdrDataTableEntry = (PLDR_DATA_TABLE_ENTRY)(pLdrDataTableEntry->InLoadOrderLinks.Flink);
	} while ((LIST_ENTRY*)pLdrDataTableEntry != pListHead);

	return nullptr;
}

void* WINAPI GetExportAddress(HMODULE hMod, const char* lpProcName) {
	char* pBaseAddress = (char*)hMod;

	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)pBaseAddress;
	IMAGE_NT_HEADERS* pNtHeaders = (IMAGE_NT_HEADERS*)(pBaseAddress + pDosHeader->e_lfanew);
	IMAGE_OPTIONAL_HEADER* pOptionalHeader = &pNtHeaders->OptionalHeader;
	IMAGE_DATA_DIRECTORY* pDataDirectory = (IMAGE_DATA_DIRECTORY*)
					(&pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	IMAGE_EXPORT_DIRECTORY* pExportDirectory = (IMAGE_EXPORT_DIRECTORY*)
					(pBaseAddress + pDataDirectory->VirtualAddress);

	void** ppFunctions = (void**)(pBaseAddress + pExportDirectory->AddressOfFunctions);
	WORD* pOrdinals = (WORD*)(pBaseAddress + pExportDirectory->AddressOfNameOrdinals);
	NULONG* pNames = (NULONG*)(pBaseAddress + pExportDirectory->AddressOfNames);
	
	void* pAddress = nullptr;

	//if we have ordinal instead of Function Name
	if (((DWORD_PTR)lpProcName >> 16) == 0) {
		WORD ordinal = LOWORD(lpProcName);
		DWORD dwOrdinalBase = pExportDirectory->Base;

		if (ordinal < dwOrdinalBase || ordinal >= dwOrdinalBase + pExportDirectory->NumberOfFunctions)
			return nullptr;

		pAddress = (FARPROC)(pBaseAddress + (DWORD_PTR)ppFunctions[ordinal - dwOrdinalBase]);
	}
	else {
		for (auto i = 0; i < pExportDirectory->NumberOfNames; i++) {
			char* szName = (char*)pBaseAddress + (DWORD_PTR)pNames[i];
			if (strcmp(lpProcName, szName) == 0) {
				pAddress = (FARPROC)(pBaseAddress + 
					((NULONG*)(pBaseAddress + pExportDirectory->AddressOfFunctions))[pOrdinals[i]]);
				//pAddress = (FARPROC)(pBaseAddress + (char*)ppFunctions[pOrdinals[i]]);
				break;
			}
		}
	}

	//Forwarded Functions
	if ((char*)pAddress >= (char*)pExportDirectory && (char*)pAddress < (char*)pExportDirectory + pDataDirectory->Size) {
		char* szDllName, * szFunctionName;
		HMODULE hForward;

		szDllName = _strdup((const char*)pAddress);
		if (!szDllName)
			return nullptr;

		pAddress = nullptr;
		szFunctionName = strchr(szDllName, '.');
		*szFunctionName++ = 0;

		typedef HMODULE(WINAPI* LoadLibraryAType)(LPCSTR lpFileName);
		LoadLibraryAType pLoadLibraryA = (LoadLibraryAType)GetExportAddress(
			GetModuleBaseAddress(L"KERNEL32.DLL"), "LoadLibraryA");

		if (!pLoadLibraryA)
			return nullptr;

		hForward = pLoadLibraryA(szDllName);
		free(szDllName);

		if (!hForward)
			return nullptr;

		pAddress = GetExportAddress(hForward, szFunctionName);
	}

	return pAddress;
}

int main() {
	//Just for loading user32.dll
	SendMessage(0, WM_KEYDOWN, 0, 0);
	
	LPCWSTR user32Name = L"User32.dll";
	typedef int(WINAPI* MessageBoxWType)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
	HMODULE user32Base = GetModuleBaseAddress(user32Name);
	if(user32Base) {
		MessageBoxWType pMessageBoxW = (MessageBoxWType)GetExportAddress(user32Base, "MessageBoxW");
		pMessageBoxW(nullptr, L"Hello from Message Box!", L"hi", MB_OK);
	}
}

