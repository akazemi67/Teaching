#include <stdio.h>

void printArray(unsigned int arr[], int n) {
	printf("Array Values:\n");
	for (int i = 0; i < n; i++) {
		printf("\t0x%X", arr[i]);
	}
	printf("\n");
}

/*
https://docs.microsoft.com/en-us/visualstudio/debugger/debugging-basics-registers-window
Flag			Set value
Sign (bit 7)	PL = 1
Zero (bit 6)	ZR = 1
*/
void asmInstructions() {
	int a[] = { 0x11223344, 0x55667788 };
	int b = 2;
	int c = 5;
	__asm {
		//Checking EFLAGS
		xor eax, eax;
		mov eax, b;
		sub eax, c;

		//LEA vs MOV? *p
		mov ebx, [a + 4];
		lea ecx, [a + 4];
	}
}

void asmReverseArray() {
	const int len = 5;
	unsigned int arr[] = { 0x11223344, 0x55667788, 0x99112233,
							0xAABBCCDD, 0xEEFF4433 };
	printArray(arr, len);
	int idx = 0;

	//Pushing array values to the stack
	__asm {
	firstLoopStart:
		mov ebx, idx;
		lea eax, [arr + ebx * 4];
		mov ebx, [eax];
		push ebx;

		inc idx;
		mov eax, idx;
		cmp eax, len;
		jl firstLoopStart;
	}

	idx = 0;
	//Poping values from stack and putting to the array
	__asm {
	secondLoopStart:
		mov ebx, idx;
		lea eax, [arr + ebx * 4];
		pop ebx;
		mov dword ptr[eax], ebx;

		inc idx;
		mov eax, idx;
		cmp eax, len;
		jl secondLoopStart;
	}

	printArray(arr, len);
}

int main() {
	asmInstructions();
	asmReverseArray();
	return 0;
}
