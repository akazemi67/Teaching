#include <Windows.h>
#include <VersionHelpers.h>
#include <cstdio>
#include <winnt.h>

/*
 * API Documentation:
 * https://learn.microsoft.com/en-us/windows/win32/devnotes/rtlgetversion
 */
OSVERSIONINFOEX GetRealWindowsVersion()
{
	//Define type for function by its signature
    typedef NTSTATUS(NTAPI* RtlGetVersionFunction)(PRTL_OSVERSIONINFOW);
	
    OSVERSIONINFOEX result;
    ZeroMemory(&result, sizeof(OSVERSIONINFOEX));
    result.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) {
        const auto pRtlGetVersion = reinterpret_cast<RtlGetVersionFunction>(GetProcAddress(ntdll, 
            "RtlGetVersion"));

        if (pRtlGetVersion) {
        	//function execution
            pRtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&result));
        }
    }

    return result;
}


/*
 * Useful Links:
 * https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa
 * https://learn.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1
 * 
 * https://stackoverflow.com/questions/11504732/how-to-add-a-manifest-file-to-a-project-in-visual-studio-2010
 */
int main() {
    OSVERSIONINFO osvi;
    BOOL bIsWindowsXPorLater;
    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
    GetVersionEx(&osvi);
	
    printf("Windows Version: %lu.%lu.%lu\n", 
        osvi.dwMajorVersion, 
        osvi.dwMinorVersion, 
        osvi.dwBuildNumber);

    printf("\nWindows 10?: %s\n", (IsWindows10OrGreater() ? "YES" : "NO"));

    printf("\n-------------------------\n\n");

    OSVERSIONINFOEX infoW = GetRealWindowsVersion();
    printf("Real Windows Version: %lu.%lu.%lu\n", 
        infoW.dwMajorVersion, 
        infoW.dwMinorVersion,
        infoW.dwBuildNumber);
	
	getchar();
    return 0;
}

