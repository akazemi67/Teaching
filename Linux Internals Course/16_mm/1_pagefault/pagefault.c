#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#define BUFFER_SIZE (4 * 1024 * 1024)

void print_pgfaults(void) {
  int ret;
  struct rusage usage;

  ret = getrusage(RUSAGE_SELF, &usage);
  if (ret == -1) {
    perror("getrusage.");
  } else {
    printf("Major page faults %ld\n", usage.ru_majflt);
    printf("Minor page faults %ld\n", usage.ru_minflt);
  }
}

int main (int argc, char *argv[]) {
  unsigned char *p;
  printf("Initial state\n");
  print_pgfaults();

  p = malloc(BUFFER_SIZE);
  printf("\nAfter malloc\n");
  print_pgfaults();

  memset(p, 'a', BUFFER_SIZE);
  printf("\nAfter memset\n");
  print_pgfaults();

  memset(p, 'b', BUFFER_SIZE);
  printf("\nAfter 2nd memset\n");
  print_pgfaults();

  getchar();
  return 0;
}



