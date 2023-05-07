#include <stdio.h>

int doSum(__int64 a, __int64 b, __int64 c, __int64 d, 
	const char *s1, char *s2) {
	printf("s1: %s\ns2: %s\n", s1, s2);
	return a + b + c + d;
}

int main() {
	char hello[] = "HelloWorld!";
	doSum(0xaaaaaaaaa, 0xbbbbbbbbb, 0xccccccccc,
		0xddddddddd, "Hi Bye!", hello);
	return 0;
}

