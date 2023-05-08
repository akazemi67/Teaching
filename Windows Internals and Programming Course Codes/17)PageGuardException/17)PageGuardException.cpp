#include <iostream>
#include <windows.h>

using namespace std;

char* data_buffer;
char* guard_page;

// Custom exception handler
LONG WINAPI CustomExceptionHandler(_EXCEPTION_POINTERS* ExceptionInfo) {
    getchar();
    // Check if the exception was caused by the guard page
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        cout << "Access violation on guard page was handled successfully" << endl;

        // Remove the guard flag from the page and continue execution
        /*DWORD oldProtect;
        if (!VirtualProtect(guard_page, 0x1000, PAGE_READWRITE, &oldProtect)) {
            cout << "Failed to remove guard flag from the page" << endl;
            return EXCEPTION_EXECUTE_HANDLER;
        }
        */
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else {
        cout << "Access violation was not caused by guard page" << endl;
        return EXCEPTION_EXECUTE_HANDLER;//previous frames
    }
}

int main() {
    // Allocate memory for the data buffer
    data_buffer = (char*)VirtualAlloc(NULL, 0x3000, 
        MEM_COMMIT, PAGE_READWRITE);
    
    // Allocate a guard page at the end of the data buffer
    guard_page = (char*)VirtualAlloc(data_buffer + 0x2000, 0x1000, 
        MEM_COMMIT, PAGE_READWRITE | PAGE_GUARD);

    printf("Data Addr: %p\nGuard Addr: %p\n", data_buffer, guard_page);
    getchar();
	
	//OK
    data_buffer[0] = 'a';
    data_buffer[0x1000] = 'b';
    printf("[0]: %c%c\n\n", data_buffer[0], data_buffer[0x1000]);
	
    // Access the guarded page within a try block
    __try {
        data_buffer[0x2100] = 'A';
    }
    // Handle access violations caused by guard pages in the except block
    __except (CustomExceptionHandler(GetExceptionInformation())) {}//Continue....

	//No More Exceptions
    data_buffer[0x2200] = 'B';
    data_buffer[0x2300] = 'C';
    printf("[0x2000]: %c%c%c\n", data_buffer[0x2100],
        data_buffer[0x2200], data_buffer[0x2300]);


    getchar();
    // Free the memory
    VirtualFree(data_buffer, 0, MEM_RELEASE);

    return 0;
}

