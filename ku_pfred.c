#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

#define MSG_SIZE 4 /* size of int */
#define MAX_PRIO 9999

/* Maximum of values = 10000 */

/* Sender */
void sender(int fd, int size, int interval, int numP, int offset) {
	struct mq_attr attr;
	int currentValue = 0;
	unsigned int prio;
	mqd_t mqdes;
	int i = 0;
	char buffer[4] = {'0', };
	int currentOffset = offset;

	attr.mq_maxmsg = 1;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open("/m_queue", O_CREAT| O_WRONLY, 0666, &attr);

	if(mqdes < 0) {
		perror("mq_open()");
		exit(0);
	}

	for(i = 0; i < size / numP; i++) {
		pread(fd, buffer, 4, currentOffset);
		currentValue = atoi(buffer);

		if(mq_send(mqdes, (char*)&currentValue, MSG_SIZE, currentValue) == -1) {
			perror("mq_send()");
		} else {
			currentOffset += 5;
		}
	}

	mq_close(mqdes);
}

/* SenderX */
void senderX(int fd, int size, int interval, int numP, int offset) {

}

void receiver(int size, int interval, int numP, int* intArr) {
	struct mq_attr attr;
	int value;
	unsigned int prio;
	mqd_t mqdes;
	int left = size;

	attr.mq_maxmsg = 1;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open("/m_queue", O_RDWR | O_CREAT, 0666, &attr);

	if(mqdes < 0) {
		perror("mq_open()");
		exit(0);
	}

	while(mq_receive(mqdes, (char*)&value, MSG_SIZE, &prio) != -1) {
		intArr[value / interval]++;
		printf("value get: %d index: %d\n", value, value / interval);
		left--;
		if(left == 0) {
			break;
		}
	}

	mq_close(mqdes);
	mq_unlink("/m_queue");
}


/* Gets the length for the first line in the file
  eg) 1000 -> 4
      10000 -> 5
*/

int getLen(int fd) {
	int len = 0;
	int offset = 0;
	char ch[1] = {'0'}; /* for size check */

	do {
		pread(fd, ch, 1, offset);
		if(ch[0] == '\n') { break; }
		else { len++; offset++; }
	} while (1);

	return len;
}

/* Gets the actual size using getLen(int)
   eg) 1000 -> size: 1000
       10000 -> size: 10000
*/
int getSize(int fd, int len) {
	char* numV = 0;
	int size = 0;
	numV = (char*) malloc(sizeof(char) * len);

	pread(fd, numV, len, 0);
	size = atoi(numV);

	free(numV);
	return size;
}

/* Prints the frequency distribution after the tasks are all done */
void printFD(int* intArr, int interval) {
	int i;

	for(i = 0; i < 10000 / interval; i++) {
		printf("%4d -- %4d: %4d \n", i * interval, (i + 1) * interval - 1, intArr[i]);
	}
}

/* Check whether the number has been counted correctly */
void FDCheck(int* intArr, int interval) {
	int i = 0;
	int sum = 0;

	for(i = 0; i < 10000 / interval; i++) {
		sum += intArr[i];
	}

	printf("total num: %d \n", sum);
}

int main(int argc, char *argv[]) {
	/* VARIABLES */
	int numP = 0; /* Number of processes */
	int interval = 0; /* Size of the interval */
	
	int fd; /* File Descriptor */
	int size; /* Number of values */
	int offset = 0; /* Indicator of where to read the file */
	int* offsetP = 0; /* Arrays of those indicators */
	int i; /* for loop */
	int remainder; /* for remainder of size / numP */

	int* intArr = 0; /* Frequency Distribution */
	pid_t* pid = 0; /* for processes */
//	char** mqArr = 0; /* for names of message queues */
	char* mqName = 0;
	int status;

	/* Gets the parameter */
	numP = atoi(argv[1]);
	interval = atoi(argv[2]);

	fd = open(argv[3], O_RDONLY);

	if(fd == -1) {
		printf("Error \n");
	}

	/* Reads the numbers of the values */
	offset = getLen(fd) + 1;
	size = getSize(fd, getLen(fd));
	remainder = size % numP;

	if(numP > size) {
		numP = size;
	}

	pid = (pid_t*) malloc(sizeof(pid_t) * numP);
//	mqArr = (char**) malloc(sizeof(char*) * numP);

	printf("size: %d, interval: %d, offset: %d\n", size, interval, offset);
	/* Dynamically allocate the fd array */
	intArr = (int*) malloc(sizeof(int) * (10000 / interval));
	memset(intArr, 0, sizeof(intArr));

	offsetP = (int*) malloc(sizeof(int) * numP);
	for(i = 0; i < numP; i++) {
		offsetP[i] = offset;
		offset += 5 * (size / numP);
	}

	if((pid[0] = fork()) == 0) {
		sender(fd, size, interval, numP, offsetP[0]);
	} else {
		receiver(size, interval, numP, intArr);
		waitpid(pid[0], &status, 0);
//		printFD(intArr, interval);
		FDCheck(intArr, interval);
	}

	close(fd);

	free(intArr);
	free(offsetP);
	free(pid);

	return 0;
}



