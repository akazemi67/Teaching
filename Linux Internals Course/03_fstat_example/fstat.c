#include <stdio.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <fcntl.h> 
#include <time.h> 
 
int main(void) 
{ 
    int f_d = 0; 
    struct stat st; 
      
    // Open the file test.txt through open() 
    // Note that since the call to open directly gives 
    // integer file descriptor so we used open here. 
    // One can also use fopen() that returns FILE* 
    // object. Use fileno() in that case to convert 
    // FILE* object into the integer file descriptor 
    f_d = open("test", O_RDONLY); 
 
    //Check if open() was successful 
    if(-1 == f_d) 
    { 
        printf("NULL File descriptor\n"); 
        return -1; 
    } 
 
    // set the errno to default value 
    errno = 0; 
    // Now a call to fstat is made 
    // Note that the address of struct stat object 
    // is passed as the second argument 
    if(fstat(f_d, &st)) 
    { 
        printf("\nfstat error: [%s]\n",strerror(errno)); 
        close(f_d); 
        return -1; 
    } 
 
    switch (st.st_mode & S_IFMT) { 
           case S_IFBLK:  printf("block device\n");            break; 
           case S_IFCHR:  printf("character device\n");        break; 
           case S_IFDIR:  printf("directory\n");               break; 
           case S_IFIFO:  printf("FIFO/pipe\n");               break; 
           case S_IFLNK:  printf("symlink\n");                 break; 
           case S_IFREG:  printf("regular file\n");            break; 
           case S_IFSOCK: printf("socket\n");                  break; 
           default:       printf("unknown?\n");                break; 
           } 
 
     printf("File size:                %lld bytes\n",(long long) st.st_size); 
     printf("Last status change:       %s", ctime(&st.st_ctime)); 
     printf("Last file access:         %s", ctime(&st.st_atime)); 
     printf("Last file modification:   %s", ctime(&st.st_mtime)); 
 
 
    // Close the file 
    close(f_d); 
 
    return 0; 
}

