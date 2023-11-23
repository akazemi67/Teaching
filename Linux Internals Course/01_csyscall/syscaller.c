#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define SYS_NUMBER  488

int main(int argc, char **argv){
    if(argc<2){
        printf("Enter a string for testing syscall.\n");
        return 1;
    }

    long res = syscall(SYS_NUMBER, argv[1]);
    if(res<0)
        printf("Error in system call.\n");
    else
        printf("All went well!\n");

    return 0;
}

