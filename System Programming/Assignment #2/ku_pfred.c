#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <signal.h>

#define MSG_SIZE 4 /* size of int */
#define MAX_PRIO 9999

/* Maximum of values = 10000 */

/* Reaps all the children */
void reaper(int sig) {
	while(waitpid(-1, 0, WNOHANG) > 0) {

	}
}

/* Sender */
/* Continuously reads the file from the given offset,
 * then sends the values continuously to the message queue */
void sender(int fd, int size, int interval, int numP, int offset, char* mqName) {
	struct mq_attr attr;
	int currentValue = 0;
	unsigned int prio;
	mqd_t mqdes;
	int i = 0;
	char buffer[4] = {'0', };
	int currentOffset = offset;

	attr.mq_maxmsg = 1;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open(mqName, O_CREAT| O_WRONLY, 0666, &attr);

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
/* When remainder != 0 sends extra messages to the message queue */
void senderX(int fd, int size, int interval, int numP, int remainder, int offset, char* mqName) {
	struct mq_attr attr;
	int currentValue = 0;
	unsigned int prio;
	mqd_t mqdes;
	int i = 0;
	char buffer[4] = {'0', };
	int currentOffset = offset;

	attr.mq_maxmsg = 1;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open(mqName, O_CREAT| O_WRONLY, 0666, &attr);

	if(mqdes < 0) {
		perror("mq_open()");
		exit(0);
	}

	for(i = 0; i < size / numP + remainder; i++) {
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

/* Opens the message queue and continuously reads the message queue */
void receiver(int size, int interval, int numP, int* intArr, char* mqName) {
	struct mq_attr attr;
	int value;
	unsigned int prio;
	mqd_t mqdes;
	int left = size / numP;

	attr.mq_maxmsg = 1;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open(mqName, O_RDWR | O_CREAT, 0666, &attr);

	if(mqdes < 0) {
		perror("mq_open()");
		exit(0);
	}

	while(mq_receive(mqdes, (char*)&value, MSG_SIZE, &prio) != -1) {
		intArr[value / interval]++;
		left--;
		if(left == 0) {
			break;
		}
	}

	mq_close(mqdes);
	mq_unlink(mqName);
}

/* When remainder != 0, main should receive more messages */
void receiverX(int size, int interval, int numP, int remainder, int* intArr, char* mqName) {
	struct mq_attr attr;
	int value;
	unsigned int prio;
	mqd_t mqdes;
	int left = size / numP + remainder;

	attr.mq_maxmsg = 1;
	attr.mq_msgsize = MSG_SIZE;

	mqdes = mq_open(mqName, O_RDWR | O_CREAT, 0666, &attr);

	if(mqdes < 0) {
		perror("mq_open()");
		exit(0);
	}

	while(mq_receive(mqdes, (char*)&value, MSG_SIZE, &prio) != -1) {
		intArr[value / interval]++;
		left--;
		if(left == 0) {
			break;
		}
	}

	mq_close(mqdes);
	mq_unlink(mqName);
}


/* To name message queues */
void nameInc(char* str) {
	 if(str[6] == '9') {
            str[6] = '0';
   
            if(str[5] == '9') {
                str[5] = '0';
   
                if(str[4] == '9') {
                    str[4] == '0';
   
                    if(str[3] == '9') {
						str[3] = '0';
						str[4] = '0';
						str[5] = '0';
						str[6] = '0';
                    } else {
                        str[3]++;
						str[4] = '0';
						str[5] = '0';
						str[6] = '0';
                    }
               } else {
                    str[4]++;
                }
            } else {
                str[5]++;
            }
        } else {
            str[6]++;
        }
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
		printf("%4d \n", intArr[i]);
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
	/* ----- VARIABLES ----- */
	int fd; /* File Descriptor */

	/* Frequency Distribution */
	int numP = 0; /* Number of processes */
	int interval = 0; /* Size of the interval */
	int size = 0; /* Number of values */
	int offset = 0; /* Indicator of where to read the file */
	int* offsetP = 0; /* Arrays of those indicators */
	int remainder = 0; /* for remainder of size / numP */

	int* intArr = 0; /* Frequency Distribution */
	
	/* Processes */
	pid_t* pid = 0; /* for processes */

	/* Message Queues */
	char** mqArr = 0; /* for names of message queues */
	char mqName[7] = "/mq0000";
	int status;

	int i; /* for loop */
	
	/* adds new handler "reaper" */
	signal(SIGCHLD, reaper);
	
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

	/* Creates message names for message queues and store them to mqArr */
	pid = (pid_t*) malloc(sizeof(pid_t) * numP);
	mqArr = (char**) malloc(sizeof(char*) * numP);

	for(i = 0; i < numP; i++) {
		mqArr[i] = (char*) malloc(sizeof(char));
		strncpy(mqArr[i], mqName, 7);
		nameInc(mqName);
	}

	/* Dynamically allocate the fd array */
	intArr = (int*) malloc(sizeof(int) * (10000 / interval));
	memset(intArr, 0, sizeof(intArr));

	offsetP = (int*) malloc(sizeof(int) * numP);
	for(i = 0; i < numP; i++) {
		offsetP[i] = offset;
		offset += 5 * (size / numP);
	}


	/* Get values from each message queue */
	for(i = 0; i < numP; i++) {
		if(remainder != 0 && i == numP - 1) {
			if((pid[i] = fork()) == 0) {
				senderX(fd, size, interval, numP, remainder, offsetP[i], mqArr[i]);
				exit(0);
			}
		} else {
			if((pid[i] = fork()) == 0) { 
				sender(fd, size, interval, numP, offsetP[i], mqArr[i]);
				exit(0);
			}
		}
	}

	/* Receives the messages from the message queue */
	for(i = 0; i < numP; i++) {
		if(remainder != 0 && i == numP - 1) {
			receiverX(size, interval, numP, remainder, intArr, mqArr[i]);
		} else {
			receiver(size, interval, numP, intArr, mqArr[i]);
		}
	}
	
	printFD(intArr, interval);
	
	/* Frees the dynamically allocated spaces and closes the file */
	close(fd);

	free(intArr);
	free(offsetP);
	free(pid);

	/* Free the mqArr** */
	for(i = 0; i < numP; i++) {
		free(mqArr[i]);
	}

	free(mqArr);

	return 0;
}



