#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include "ch3.h"

int dev;
struct msg_st user_buf;


void msg_list_read() {
	memset(user_buf.str, 0, sizeof(user_buf.str));
	user_buf.len = 0;
	ioctl(dev, CH3_IOCTL_READ, &user_buf);
	printf("ch3_app: msg - %s\n", user_buf.str);
}

void msg_list_write(const char* str) {
	memset(user_buf.str, 0, sizeof(user_buf.str));
	strcpy(user_buf.str, str);
	user_buf.len = strlen(user_buf.str);
	ioctl(dev, CH3_IOCTL_WRITE, &user_buf);
	printf("ch3_app: wrote msg - %s\n", user_buf.str);
}

int main(void) {
	dev = open("/dev/ch3_dev", O_RDWR);

	msg_list_read();
	msg_list_write("banana");
	msg_list_write("hello");
	msg_list_read();
	msg_list_read();
	msg_list_read();

	close(dev);
}
