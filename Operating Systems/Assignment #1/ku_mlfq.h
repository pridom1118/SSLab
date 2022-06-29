#ifndef KU_MLFQ_H
#define KU_MLFQ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#define GAMING_TOLERANCE 2 // Game Tolerance
#define PRIORITY_BOOST 10 // S for Priority Boost
#define MAX_PRIORITY 2

typedef struct pcb {
    pid_t pid;
    double arrivalTime;
    int pname, priority, timeAllot;
}PCB;

/* Queue Implementation with Linked List */
typedef struct node {
    PCB data;
    struct node* next;
}Node;

typedef struct queue {
    struct node* front, * rear;
    int size;
}Queue;

int isEmpty(Queue* q) {
    return q->size == 0;
}

void initQueue(Queue* q) {
    q->front = q->rear = NULL;
    q->size = 0;
}

void enqueue(Queue* q, PCB e) {
    Node* newNode = malloc(sizeof(Node));
    newNode->data = e;

    if(isEmpty(q)) q->front = q->rear = newNode;
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

PCB dequeue(Queue* q) {
    if (isEmpty(q)) exit(1);
    PCB f = q->front->data;
    Node* front = q->front;
    q->front = front->next;
    free(front);
    front = NULL;
    q->size--;
    return f;
}

/* for system arrival time */
double getTime() {
    struct timeval t;
    gettimeofday(&t, (void*)0);
    return (double)t.tv_sec + (double)t.tv_usec / 1e6;
}

#endif //KU_MLFQ_H
