#include <cstdio>
#include <cstring>
#include <Windows.h>

//To turn off the warning globally in command-line builds, use the /wd4996 command-line option.
#pragma warning(disable : 4996)

unsigned int __stdcall funcStd(	unsigned int a, unsigned int b, const char s1[], 
								unsigned int c, unsigned int d, const char s2[]) {
	unsigned int eLocal = 0xeeeffeee;
	char sLocal[] = "Hello Std";
	printf("Strings are: %s %s %s\n", sLocal, s1, s2);
	return a + b + c + d + eLocal;
}

unsigned int __cdecl funcCdecl(	unsigned int a, unsigned int b, const char s1[],
								unsigned int c, unsigned int d, const char s2[]) {
	unsigned int eLocal = 0xeeeffeee;
	char sLocal[] = "Hello Cdecl";
	printf("Strings are: %s %s %s\n", sLocal, s1, s2);
	return a + b + c + d + eLocal;
}

int anotherLevel(unsigned int x, unsigned int y) {
	int a = x + y;
	int b = 0;
	return a / b;
}

void innerCall(unsigned int par1, unsigned int par2, unsigned int par3) {
	int a = par1 + par2;
	int b = par3 + par2;
	anotherLevel(a, b);
}

unsigned int __fastcall funcFast(	unsigned int a, unsigned int b, const char s1[],
									unsigned int c, unsigned int d, const char s2[]) {
	unsigned int eLocal = 0xeeeffeee;
	char sLocal[] = "Hello Fast";
	printf("Strings are: %s %s %s\n", sLocal, s1, s2);
	innerCall(a, b, c);
	return a + b + c + d + eLocal;
}
 __int64 func64(__int64 a, __int64 b, const char s1[],
				__int64 c, __int64 d, const char s2[]) {
	 __int64 eLocal = 0xeeeffeee;
	char sLocal[] = "Hello Std";
	printf("Strings are: %s %s %s\n", sLocal, s1, s2);
	return a + b + c + d + eLocal;
}

typedef struct _DummyStruct {
	int ii;
	short ss;
	char cc;
	char cc2;
}DummyStruct;

typedef struct _DummyBytes {
	char bytes[10];
} DummyBytes;

typedef struct _MyStruct {
	int a;
	int b;
	DummyStruct *ds;
	char s[20];
} MyStruct;

void funcDataValue(MyStruct stDat) {
	stDat.a += 0x10;
	stDat.b += 0x20;
	printf("\nValues %x %x %s\n", stDat.a, stDat.b, stDat.s);
}

void funcDataRef(MyStruct &stDat) {
	stDat.a += 0x10;
	stDat.b += 0x20;
	printf("\nValues %x %x %s\n", stDat.a, stDat.b, stDat.s);
}

void f() {
	printf("This should never be called!!!\n");
}

MyStruct *globalStruct;

int main() {
	/*
	void* a;
	int* b;
	char* c;
	short* d;
	MyStruct* s;
	*/
	DummyBytes db;
	int x = 0xbb;
	char sLocal[10];
	strcpy(sLocal, "Hello!");
	
	auto* dumm = new DummyStruct{0x6c6c6548, 0x486f, 'i'};
	auto* ms = new MyStruct{ 1111, 5555, dumm, "HelloHi" };
	globalStruct = ms;
	
	printf("StdCall Sample: \n\t");
	funcStd(0xaaaffaaa, 0xbbbffbbb, "Str111", 0xcccffccc, 0xdddffddd, "Str222");
	printf("CdeclCall Sample: \n\t");
	funcCdecl(0xaaaffaaa, 0xbbbffbbb, "Str111", 0xcccffccc, 0xdddffddd, "Str222");
	printf("FastCall Sample: \n\t");
	funcFast(0xaaaffaaa, 0xbbbffbbb, "Str111", 0xcccffccc, 0xdddffddd, "Str222");
	
	MyStruct stData;
	stData.a = 0xaaaa;
	stData.b = 0xbbbb;
	strcpy(stData.s, "This Data DDD");
	
	funcDataValue(stData);
	printf("\tAfter Value Call: %x %x\n", stData.a, stData.b);
	funcDataRef(stData);
	printf("\tAfter Ref Call: %x %x\n", stData.a, stData.b);

	//Sample for easier x64 calling convention checking 
	func64(	0xaaaaffffaaaa, 0xbbbbffffbbbb, "Str64_111", 
			0xccccffffcccc,0xddddffffdddd, "Str64_222");
	
	return 0;
}

