#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <memory.h>

#include "ku_binder.h"

int fd;

int kbinder_init(void) {
	fd = open("/dev/ku_binder_dev", O_RDWR);
	return 0;
}

/* APIs for Service Clients */

int kbinder_query(char *sname) {
	int ret;

	ret = ioctl(fd, BINDER_QUERY, sname);
	
	if(ret == -1) {
		printf("client: %s is not registered yet\n", sname);
		return -1;
	} else {
		printf("client: %s found, with service number %d\n", sname, ret);
		return ret;
	}
	return -1;
}

int kbinder_rpc(int snum, int fcode, void *param) {
	int ret = -1;
	
	struct binder_msg my_msg;
	
	my_msg.snum = snum;
	my_msg.fcode = fcode;
	my_msg.param = *(union param*) param;
	ret = ioctl(fd, BINDER_RPC, &my_msg);

	return ret;
}

/* APIs for Service Servers */
int kbinder_reg(char *sname) {
	int ret = -1;
	ret = ioctl(fd, BINDER_REG, sname);
	
	if(ret == -1) {
		printf("server: cannot register the service %s\n", sname);
		return -1;
	} else {
		printf("server: %s successfully registered with service number %d\n", sname, ret);
		return ret;
	}
	return -1;
}

int kbinder_read(int snum, void* buf) {
	int ret = -1;
	//TODO: somehow arrange the buf so that you can store the data.
	struct binder_msg my_msg;
	my_msg.snum = snum;
	ret = ioctl(fd, BINDER_READ, &my_msg);

	if(ret != -1) {
		((struct msg*)buf)->fcode = my_msg.fcode;
		((struct msg*)buf)->param = my_msg.param;
	}

	return ret;
}
