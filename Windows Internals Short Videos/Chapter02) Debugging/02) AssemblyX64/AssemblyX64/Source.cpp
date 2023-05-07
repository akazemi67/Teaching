#include <cstdio>

extern "C" long copy_str(char* src, char* dst);

int main() {
	char s1[50] = "Hello World!";
	char s2[50] = {0};
	printf("Before:\nS1: %s\nS2: %s\n\n\n", s1, s2);

	int ret = copy_str(s1, s2);
	printf("After\nS1: %s\nS2: %s\n", s1, s2);
	printf("Ret: %d\n", ret);
	
	return 0;
}

