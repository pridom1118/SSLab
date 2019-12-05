#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/* GLOBAL VARIABLES */
int* intArr; /* Frequency Distribution */
pthread_mutex_t mutex; /* for blocking simultaneous accesses to critical section */

/*GLOBAL VARIABLES BUT NOT IN CRITICAL SECTION */
int FD; /* For opening dataset */
int INTERVAL; /* Size of an interval */
int SIZE; /* Number of values */
int NUMT; /* Number of threads */

volatile int REMAINDER; /* To check whether there are remainders in SIZE / NUMT */

/* a function for thread */
/* input: an offset 
   reads numbers in the file for the given part
   starting at the given offset 
*/
void *readT(void *data) {
	int i;
	int currentValue;
	int currentOffset = *((int*)data);
	int index = 0;
	char buffer[4] = {'0', }; /* temporary buffer for reading values */

	for(i = 0; i < SIZE / NUMT; i++) {
		pread(FD, buffer, 4, currentOffset);
		currentValue = atoi(buffer);
		
		pthread_mutex_lock(&mutex);

		intArr[atoi(buffer) / INTERVAL]++;
		
		pthread_mutex_unlock(&mutex);

		currentOffset += 5;
	}

	return NULL;
}

/* When there are remainders in SIZE / NUMT, the thread created at last
   will calculate the frequency distribution to compensate for those remainders */

void *readTX(void *data) {
	int i;
	int currentValue;
	int currentOffset = *((int*)data);
	int index = 0;
	char buffer[4] = {'0', }; /* temporary buffer for reading values */

	for(i = 0; i < SIZE / NUMT + REMAINDER; i++) {
		pread(FD, buffer, 4, currentOffset);
		currentValue = atoi(buffer);
		
		pthread_mutex_lock(&mutex);

		intArr[atoi(buffer) / INTERVAL]++;

		pthread_mutex_unlock(&mutex);

		currentOffset += 5;
	}

	return NULL;

}

/* for checking whether the numbers have been recorded properly */
void FDCheck() {
	int i;
	int sum = 0;

	for(i = 0; i < 10000 / INTERVAL; i++) {
		sum += intArr[i];
	}

	printf("total num: %d\n", sum);
}

/* Prints the frequency distribution after the tasks are all done */
void printFD() {
	int i;
	
	for(i = 0; i < 10000 / INTERVAL; i++) {
		printf("%4d -- %4d: %4d\n", i * INTERVAL, (i + 1) * INTERVAL - 1, intArr[i]);
	}
}

/* Gets the length for the first line in the file which is the number of data */
/* eg) if the first line of the file is 1000 - then it will return strlen("1000") 
       which is 4 
*/
int getLen() {
	int len = 0;
	int offset = 0;
	char ch[1] = {'0'}; /* for size check */

	do {
		pread(FD, ch, 1, offset);
		if(ch[0] == '\n') { break; }
		else { len++; offset++; }
	} while (1);
	 
	return len;
}

/* Gets the actual size (or the number of data) using getLen() */
/* if the first line says 1000, then 1000 */
int getSize(int len) {
	char* numV = 0;
	int size = 0;
	numV = (char*) malloc(sizeof(char) * len);

	pread(FD, numV, len, 0);
	size = atoi(numV);

	free(numV);
	return size;
}


int main(int argc, char *argv[]){
	/* Variables */
    int offset; 
	int* offsetT; /* an array for storing each offsets that threads will be reading from */
	int i; /* for loop statement */

	/* Threads */
	pthread_t* p_thread;
	int* thr_id;

	/* puts arguments' values to varaibles */
	NUMT = atoi(argv[1]); 
	INTERVAL = atoi(argv[2]);

	/* Opens the file "dataset" */
	FD = open(argv[3], O_RDONLY);

	if(FD == -1) {
		perror("open()");
	}

	/* Reads the numbers of the values */
	offset = getLen() + 1;
	SIZE = getSize(getLen());
	REMAINDER = SIZE % NUMT;

	/* If the number of threads are bigger than the size, then 
	 * threads will be created up to 'SIZE'
	 */
	if(NUMT > SIZE) { 
//		printf("Number of threads are bigger than the size of the data \n");
		NUMT = SIZE;
//		printf("Current number of threads changed to %d\n", NUMT);
	}

	/* Initalizes the mutex */
	pthread_mutex_init(&mutex, NULL);

	/* initializes frequency distribution */
	intArr = (int*) malloc(sizeof(int) * (10000 / INTERVAL));
	memset(intArr, 0, sizeof(intArr));

	/* using threads */
	p_thread = (pthread_t*)malloc(sizeof(pthread_t) * NUMT);
	thr_id = (int*) malloc(sizeof(int) * NUMT);

	/* Initialize offset array */
	offsetT = (int*) malloc(sizeof(int) * NUMT);
	for(i = 0; i < NUMT; i++) {
		offsetT[i] = offset;
		/* an = a1 + 5(n - 1), a1 = offset */
		offset += 5 * (SIZE / NUMT); 
	}
		

/* Create threads for NUMT times */	
	for(i = 0; i < NUMT; i++) {
		if(REMAINDER != 0 && i == NUMT - 1) {
			thr_id[i] = pthread_create(&p_thread[i], NULL, readTX, (void*) &offsetT[i]);
		} else {
            thr_id[i] = pthread_create(&p_thread[i], NULL, readT, (void*) &offsetT[i]);
		}
        
		if(thr_id[i] < 0) {
			perror("pthread_create()");
			exit(0);
		}
	}

	/* Reaping */
	for(i = 0; i < NUMT; i++) {
		pthread_join(p_thread[i], NULL);
	} 
	
	/* destroy used mutex */
	pthread_mutex_destroy(&mutex);
	
	printFD();
//	FDCheck();

	close(FD);

	/* Free all dynamically allocated memories */
	free(intArr);
	free(p_thread);
	free(thr_id);
	free(offsetT);

	return 0;
}
