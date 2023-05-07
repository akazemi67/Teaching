// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <cstdio>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH) {
        char msg[1024], procName[256];
        GetModuleFileNameA(NULL, procName, sizeof(procName));
        sprintf_s(msg, "Showing MessageBox from processId %lu \n"
            "with path: %s", 
            GetCurrentProcessId(), procName);
        MessageBoxA(NULL, msg, "Msg", MB_OK);
    }
    return TRUE;
}

