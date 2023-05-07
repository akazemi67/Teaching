#include <ntddk.h>

/*
sc create ModuleName type= kernel binPath= PathToModule.sys
 */
void HelloUnloadRoutine(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	DbgPrint("Mahsan: Hello Module Unloaded!\n");
}

extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath) {

	DbgPrint("Mahsan: Hello Module Started!\n");
	DbgPrint("Mahsan: Hello Module Registry Path: %wZ\n", RegistryPath);
	
	DriverObject->DriverUnload = HelloUnloadRoutine;
	return 0;
}
