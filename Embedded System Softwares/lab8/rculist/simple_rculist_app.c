#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define READ 1
#define UPDATE 2
#define READALL 3

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ  _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_UPDATE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define IOCTL_READALL _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)

int main(int argc, char *argv[]) {
	int dev;
	int i, id, op;

	if(argc != 3) {
		printf("Two arguments required\n");
		printf("First argument: Read / Update / READALL \n");
		printf("Second argument: id\n");
		return -1;
	}

	op = atoi(argv[1]);
	id = atoi(argv[2]);

	dev = open("/dev/simple_rculist_dev", O_RDWR);

	if(op == READ) ioctl(dev, IOCTL_READ, (unsigned long) id);
	else if(op == UPDATE) ioctl(dev, IOCTL_UPDATE, (unsigned long) id);
	else if(op == READALL) ioctl(dev, IOCTL_READALL, (unsigned long) 0);
	else {
		printf("Invalid OP!\n");
		close(dev);
		return 0;
	}

	close(dev);
	return 0;
}
