#include <stdio.h>

void func(int a, int b, char leak_me[]){
    char inp[100];
    fgets(inp, 100, stdin);
    printf(inp);
    puts("Bye bye!\n");
}

int main(){
    func(0xaabbcc, 0xddeeff, "LEAKED DATA: hello");
    printf("Inside main\n");
    return 0;
}

