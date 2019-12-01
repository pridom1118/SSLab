#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

/* Max number of values = 10000 */

int* createArr(int* intArr, int size, int interval) {
	/* Creating Array based on size and interval */
	intArr = (int*)malloc(sizeof(int) * (10000 / interval));
	memset(intArr, 0, sizeof(intArr));
	return intArr;
}

void printFD(int* intArr, int interval) {
	int i;

	for(i = 0; i < 10000 / interval; i++) {
		printf("%4d -- %4d: %4d \n", i * interval, (i + 1) * interval - 1, intArr[i]);
	}
}

int main(int argc, char *argv[]) {
	int proc_num = 0;
	int interval = 0;
	
	int fd;
	int size;
	int offset = 0;
	int i;

	char buffer[4] = {'\0'};

	int* intArr = 0; /* Frequency Distribution */

	proc_num = atoi(argv[1]);
	interval = atoi(argv[2]);

	fd = open(argv[3], O_RDONLY);

	if(fd == -1) {
		printf("Error \n");
	}

	pread(fd, buffer, 4, 0); 

	size = atoi(buffer);

	intArr = createArr(intArr, size, interval);

	for(i = 0; i < size; i++) {
		pread(fd, buffer, 4, offset);
		intArr[atoi(buffer) / interval]++;
		offset += 5;
	}

	printFD(intArr, interval);

	int sum = 0;

	for(i = 0; i < 10000 / interval; i++) {
		sum += intArr[i];
	}

	printf("num: %d \n", sum);

	return 0;
}



