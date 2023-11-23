#include <stdio.h>

int main(){
    printf("Executing cli from assembly.\n");
    fflush(0);
    asm("cli");

    printf("After Execution....\n");
    fflush(0);
    return 0;
}
