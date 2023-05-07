#include <Windows.h>
#include <cstdio>

constexpr auto DeviceName = L"\\\\.\\ProcManSymlink";

enum class ProcManIoctls {
	HideProcces = CTL_CODE(0x8000, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS),
	Elevate = CTL_CODE(0x8000, 0x802, METHOD_NEITHER, FILE_ANY_ACCESS)
};

struct ParametersFromUser {
	ULONG pid;
};

bool SendDeviceCommand(HANDLE hDevice, ProcManIoctls command, ParametersFromUser params) {
	DWORD ret;
	bool status = DeviceIoControl(
		hDevice,
		static_cast<DWORD>(command),
		&params, sizeof(params),
		nullptr, 0,
		&ret, nullptr);
	if (!status) {
		printf("Error in communicating with the device: %lx\n", GetLastError());
		return false;
	}
	return true;
}

int main(int argc, const char* argv[]) {
	if(argc<3) {
		printf("Usage: client.exe elevate|hide PID");
		return 1;
	}

	ULONG pid = atol(argv[2]);
	const auto hDevice = CreateFile( DeviceName,
		GENERIC_WRITE,FILE_SHARE_WRITE,
	    nullptr,OPEN_EXISTING,
	    0, nullptr);
	if(hDevice == INVALID_HANDLE_VALUE) {
		printf("Error accessing driver. code: 0x%lx\n", GetLastError());
		return 1;
	}

	ParametersFromUser params = { pid };
	if(!strcmp("elevate", argv[1])) {
		printf("Elevating process with id: %ld\n", pid);
		if(SendDeviceCommand(hDevice, ProcManIoctls::Elevate, params)) {
			printf("Successfully sent Elevate command for pid: %ld\n", pid);
		}
	}
	else if (!strcmp("hide", argv[1])) {
		printf("Hiding process with id: %ld\n", pid);
		if (SendDeviceCommand(hDevice, ProcManIoctls::HideProcces, params)) {
			printf("Successfully sent HideProcess command for pid: %ld\n", pid);
		}
	}
	else {
		printf("Invalid command!\n");
	}
	
	CloseHandle(hDevice);
	return 0;
}
