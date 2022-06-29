#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <time.h>
#include <stdint.h>

// DEFINE
#define MSG_SIZE 4
#define MQ_NAME "/m_qu011"

#ifdef DEBUG
    #define MODE 1 // 0: 일반, 1: DEBUG
#else
    #define MODE 0
#endif