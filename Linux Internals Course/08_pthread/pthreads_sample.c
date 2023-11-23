#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
  
int x=1;

void* func(void* arg) {  
    printf("Inside the thread: %d\n", ++x); 
    
    getchar();
    // exit the current thread 
    pthread_exit(NULL); 
} 

int main() { 
    pthread_t ptid; 

    // Creating a new thread 
    pthread_create(&ptid, NULL, &func, NULL); 
  
    // Waiting for the created thread to terminate 
    pthread_join(ptid, NULL); 
    
    printf("Inside main: %d\n", x);
    return 0; 
} 

