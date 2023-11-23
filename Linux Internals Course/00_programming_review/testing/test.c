#include <stdio.h>

#define For(i,n) for(int i=0; i<n; i++)

int global;
unsigned char data[] = {0xaa, 0xbb, 0xcc, 0xdd};

int main(){
    global ++;
    int local1 = * (int*) data;
    printf("Printing some strings....\n");
    
    char str[] = "HeLlO: !";
    local1=global;
    For(i, 5)
        printf("Str %s: %d\n", str, i);

    getchar();
    return local1;
}

