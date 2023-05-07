#include <ntddk.h>
#define MILLISECOND 10000
#define RELATIVE_MILLISECOND (-MILLISECOND)

PETHREAD threadObject;
bool killThread;

VOID SampleThreadRoutine(PVOID param) {
	UNREFERENCED_PARAMETER(param);
	LARGE_INTEGER sleepTime;
	auto i = 0;

	while(++i){
		if (killThread) 
			break;
		
		sleepTime.QuadPart = 1000 * RELATIVE_MILLISECOND;
		KeDelayExecutionThread(KernelMode, /*Specifies the processor mode
			in which the caller is waiting, which can be either KernelMode
			or UserMode. Lower-level drivers should specify KernelMode.*/

			FALSE, /*Specifies TRUE if the wait is alertable. 
			Lower-level drivers should specify FALSE.*/

			&sleepTime /*Specifies the absolute or relative time, 
			in units of 100 nanoseconds, for which the wait is to occur.
			A negative value indicates relative time.

			Note: System time is a count of 100-nanosecond intervals
			since January 1, 1601. */
		);
		DbgPrint("AKazemi: Kernel Thread round: %d\n", i);
	}

	KdPrint(("AKazemi: Kernel Thread finished execution.\n"));
	PsTerminateSystemThread(STATUS_SUCCESS);
}

void KernelThreadDriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	killThread = true;
	
	if (threadObject) {
		KeWaitForSingleObject(threadObject, /*Pointer to an initialized 
			dispatcher object (event, mutex, semaphore, thread, or timer)*/
			
			Executive, //A driver should set this value to Executive
			KernelMode, //Lowest-level drivers should specify KernelMode
			FALSE,
			nullptr /*Pointer to a time-out value 
			that specifies the absolute or relative time, 
			in 100-nanosecond units, at which the wait is to be completed.*/
		);
		ObDereferenceObject(threadObject);
	}
	
	DbgPrint("AKazemi: KernelThread Driver Unload.\n");
}

extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	killThread = false;
	
	HANDLE threadHandle;
	NTSTATUS status = PsCreateSystemThread(
		&threadHandle, /*Points to a variable that will receive the handle. 
					   The driver must close the handle with ZwClose
					   once the handle is no longer in use.*/
		GENERIC_ALL,/*represents the requested types of access 
					to the created thread.*/
		nullptr,
		nullptr,
		nullptr,
		SampleThreadRoutine,
		nullptr //argument that is passed to the thread
	);

	if (!NT_SUCCESS(status)) {
		DbgPrint("AKazemi: Error creating kernel thread.\n");
		return status;
	}

	status = ObReferenceObjectByHandle(threadHandle,
		THREAD_ALL_ACCESS, //Specifies the requested types of access to the object.
		nullptr, /*ObjectType can be 
				 *ExEventObjectType, *ExSemaphoreObjectType,
				 *IoFileObjectType, *PsProcessType, *PsThreadType, */

		KernelMode,/*Specifies the access mode to use for the access check. 
				It must be either UserMode or KernelMode.
				Drivers should always specify UserMode
				for handles they receive from user address space.*/

		reinterpret_cast<PVOID*>(&threadObject), /*
			Pointer to a variable that receives a pointer to the object's body. */

		nullptr //Drivers set this to NULL.
	);
	ZwClose(threadHandle);
	
	if(!NT_SUCCESS(status)) {
		DbgPrint("AKazemi: Error getting thread object.");
		return status;
	}
	
	DbgPrint("AKazemi: Driver initialized. Thread ID: %lu\n",
		PsGetThreadId(threadObject));
	
	DriverObject->DriverUnload = KernelThreadDriverUnload;
	return STATUS_SUCCESS;
}
