#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

#define MSG_SIZE 4
#define MAX_PRIO 10000
/* Max number of values = 10000 */

int* createArr(int* intArr, int size, int interval) {
	/* Creating Array based on size and interval */
	intArr = (int*)malloc(sizeof(int) * (10000 / interval));
	memset(intArr, 0, sizeof(intArr));
	return intArr;
}


/* Prints the frequency distribution */
void printFD(int* intArr, int interval) {
	int i;

	for(i = 0; i < 10000 / interval; i++) {
		printf("%4d -- %4d: %4d \n", i * interval, (i + 1) * interval - 1, intArr[i]);
	}
}

/* Checks whether the input has given correctly */
void checkNum(int* intArr, int interval) {
    int i;
	int sum = 0;

	for(i = 0; i < 10000 / interval; i++) {
		sum += intArr[i];
	}

	printf("num: %d \n", sum);
}

int readFile(int fd, int offset) {
	int value = 0;
	char buffer[4] = {'0', };

	pread(fd, buffer, 4, offset);
	
	value = atoi(buffer);
	return value;
}

void writeFD(mqd_t mqdes, int value, int interval) {
	int index = value / interval;
	
	if(mq_send(mqdes, (char*)&index, MSG_SIZE, index) == -1) {
		perror("mq_send()");
	}
}

int main(int argc, char *argv[]) {
	int proc_num = 0;
	int interval = 0;
	
	int fd; // for opening a file (dataset)
	int size;
	int offset = 0;
	int i; //for loop

	char buffer[4] = {'\0'};

	int* intArr = 0; /* Frequency Distribution */

	/* msgqueue */
	struct mq_attr attr;
	int value;
	unsigned int prio;
	mqd_t mqdes;
	int* receiv = 0;

	/* Initialization */
	proc_num = atoi(argv[1]); //gets number of processes
	interval = atoi(argv[2]); //gets the size of interval

	fd = open(argv[3], O_RDONLY);

	if(fd == -1) {
		printf("Error \n");
	}

	pread(fd, buffer, 4, 0); 

	size = atoi(buffer);

	intArr = createArr(intArr, size, interval);

	if(fork() == 0) {
		struct mq_attr attr;
		mqd_t mqdes;
		int value = 0;
		int offset = 4;
		int* fdArr = (int*)malloc(sizeof(int) * size);
		int fd_i = 0;

		attr.mq_maxmsg = 1;
		attr.mq_msgsize = 8;


		mqdes = mq_open("/m_queue1", O_CREAT | O_WRONLY, 0666, &attr);

		if(mqdes < 0) {
			perror("mq_open()");
			exit(0);
		}

		for(i = 0; i < size; i++) {
			value = readFile(fd, offset);
//			writeFD(mqdes, value, interval);
			fdArr[fd_i++] = value / interval;
			
			if(mq_send(mqdes, (char*)fdArr, 8, 0) == -1) {
				perror("mq_send()");
			} else {
				offset += 5;
			}
		}

		mq_close(mqdes);
	}


	attr.mq_maxmsg = 1;
	attr.mq_msgsize = 8;

	mqdes = mq_open("/m_queue1", O_RDWR | O_CREAT, 0666, &attr);

	if(mqdes < 0) {
		perror("open()");
		exit(0);
	}

	while(mq_receive(mqdes, (char*)&value, MSG_SIZE, &prio) != -1) {
		mq_getattr(mqdes, &attr);
		receiv = &value;
		if(attr.mq_curmsgs == 0) {
			break;
		}
	}

	mq_close(mqdes);

		
	/* Part where it adds value to intArr */
	for(i = 0; i < size; i++) {
		intArr[receiv[i]]++;
	}
	/*
	for(i = 0; i < size; i++) {
		pread(fd, buffer, 4, offset);
		intArr[atoi(buffer) / interval]++;
		offset += 5;
	}
	*/

	printFD(intArr, interval);

	return 0;
}



