#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
 
int main()
{
    int x = 1;
    
    pid_t p = fork();
    if(p<0){
      perror("fork fail");
      exit(1);
    }
    else if (p == 0){
        printf("Child (%d) has x = %d\n", getpid(), ++x);
        getchar();
    }
    else{
        printf("Parent (%d) has x = %d\n", getpid(), --x);
        getchar();
    }
    return 0;
}


