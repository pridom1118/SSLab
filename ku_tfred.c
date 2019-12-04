#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/* GLOBAL VARIABLES */
int* intArr; /* Frequency Distribution */
pthread_mutex_t mutex, mutex2; /* for blocking concurrent access to critical section */

/*GLOBAL VARIABLES BUT NOT IN CRITICAL SECTION */
int FD;
//int offset;
int INTERVAL;
int SIZE;
int NUMT;

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

	printf("current offset: %d\n", currentOffset);
	for(i = 0; i < SIZE / NUMT; i++) {
		pread(FD, buffer, 4, currentOffset);
		currentValue = atoi(buffer);
		
		pthread_mutex_lock(&mutex);

		intArr[atoi(buffer) / INTERVAL]++;

//      printf("%d: %s\n", atoi(buffer) / INTERVAL, buffer); 
		pthread_mutex_unlock(&mutex);

		currentOffset += 5;
	}

	return NULL;
}

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
	int* offsetT;
	char buffer[4] = {'0', }; 
	int i;

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
		/* an = a1 + 5(n - 1) */
		offset += 5 * SIZE / NUMT; 
	}
		

	/* Create threads for NUMT times */
	for(i = 0; i < NUMT; i++) {
		thr_id[i] = pthread_create(&p_thread[i], NULL, readT, (void*) &offsetT[i]);
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
	FDCheck();

	close(FD);

	/* Free all dynamically allocated memories */
	free(intArr);
	free(p_thread);
	free(thr_id);
	free(offsetT);

	return 0;
}
