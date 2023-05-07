#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../include/mah_char.h"

#define DEVICE_PATH	"/dev/mah_cdev"

static void error(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

static void usage(const char *argv0) {
	printf("Usage: %s <options>\n options:\n"
			"\tp - print\n"
			"\td - down\n"
			"\tu - up\n", argv0);
	exit(EXIT_FAILURE);
}

/*
 * Sample run:
 *  ./mah_cdev_test p		; print ioctl message
 */
int main(int argc, char **argv) {
	int fd;
	char buffer[BUFFER_SIZE];

	if (argc < 2)
		usage(argv[0]);

	if (strlen(argv[1]) != 1)
		usage(argv[0]);

	fd = open(DEVICE_PATH, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	switch (argv[1][0]) {
	case 'p':				
		if (ioctl(fd, MY_IOCTL_PRINT, 0) < 0) {
			perror("ioctl");
			exit(EXIT_FAILURE);
		}

		break;
	case 'd':				
		if (ioctl(fd, MY_IOCTL_DOWN, 0) < 0) {
			perror("ioctl");
			exit(EXIT_FAILURE);
		}
		break;
	case 'u':			
		if (ioctl(fd, MY_IOCTL_UP, 0) < 0) {
			perror("ioctl");
			exit(EXIT_FAILURE);
		}
		break;
	default:
		error("Wrong parameter");
	}

	close(fd);

	return 0;
}


