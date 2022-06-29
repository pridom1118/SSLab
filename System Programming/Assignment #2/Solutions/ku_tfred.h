#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
#include <stdint.h>

// GLOBAL VARIABLE
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int* arr;
int* read_cnt;
int fd; 
int interval;
int rank;
int start_pnt;

// FUNCTION
void* thread_function(void* data);