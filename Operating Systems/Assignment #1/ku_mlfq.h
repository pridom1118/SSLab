#include "ku_mlfq.h"

/* Global variables */
volatile int alarmCount = 0;
volatile int runningTime = 0;
volatile int processNum = 0;
volatile int recentProc = -1;
pid_t *pidList = NULL;

/* Multi-Level Queues */
/* readyQueue[0]: lowest from readyQueue[2]: highest */
/* so the priorities of processes would be 0 ~ 2 */
Queue readyQueue[3];

int max(int a, int b) {
    if(a >= b) return a;
    else return b;
}

/* handler for SIGALRM, has MLFQ policy */
void scheduler(int sig) {
    if(alarmCount > runningTime) {
        struct itimerval value;
        /* Disable the real time interval timer */
        value.it_value.tv_sec = 0;
        value.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &value, NULL);
        kill(recentProc, SIGSTOP);
    } else {
        /* Stop currently running process */
        if(recentProc != -1) {
            kill(recentProc, SIGSTOP);
        }

        PCB cur; //Running process for the current time slice
        int isAllEmpty = 1;
        /* Run processes */
        for(int i = 2; i >= 0; i--) {
            if(isEmpty(&readyQueue[i])) continue;
            isAllEmpty = 0;
            cur = dequeue(&readyQueue[i]);
            kill(cur.pid, SIGCONT);
            recentProc = cur.pid;
            cur.timeAllot++;

            /* If time allotment reaches gaming tolerance */
            if(cur.timeAllot >= GAMING_TOLERANCE) {
                cur.timeAllot = 0;
                cur.priority = max(cur.priority - 1, 0);
            }
            enqueue(&readyQueue[cur.priority], cur);
            break;
        }

        alarmCount++;

        /* Priority Boost */
        /* Find the queue that is not empty
         * then boost all the jobs
         * excluding currently ran job with its time allotment fulfilled at the topmost queue */
        if(alarmCount % PRIORITY_BOOST == 0 && !isAllEmpty) {
            for(int i = 1; i >= 0; i--) {
                int currentSize = readyQueue[i].size;
                for(int j = 0; j < currentSize; j++) {
                    PCB p = dequeue(&readyQueue[i]);
                    //If the process is already ran this time do not boost the priority
                    if(p.pid == cur.pid && cur.timeAllot == 0 && cur.priority == MAX_PRIORITY - 1) {
                        enqueue(&readyQueue[p.priority], p);
                        continue;
                    }
                    p.timeAllot = 0;
                    p.priority = MAX_PRIORITY;
                    enqueue(&readyQueue[p.priority], p);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3){
        printf("ku_mlfq: Wrong number of arguments\n");
        exit(1);
    }

    processNum = atoi(argv[1]);  //Number of processes
    runningTime = atoi(argv[2]); //Given number of time slices to run
    pidList = malloc(sizeof(pid_t) * processNum); //for terminating child processes after scheduling is finished

    if(processNum > 26 || processNum < 1) {
        fprintf(stderr, "ku_mlfq: Invalid argument value\n");
        exit(1);
    }

    /* Ready queue initialization */
    initQueue(&readyQueue[0]);
    initQueue(&readyQueue[1]);
    initQueue(&readyQueue[2]);

    double curTime = getTime();

    /* Process creation */
    for(int i = 0; i < processNum; i++) {
        char pName[2] = {'A' + i, '\0'};
        PCB currentProcess;

        int rc = fork();
        if(rc == 0) {
            execl("ku_app", "ku_app", pName, NULL);
        } else {
            currentProcess.arrivalTime = getTime() - curTime;
            currentProcess.pname = i;
            currentProcess.priority = 2;
            currentProcess.timeAllot = 0;
            pidList[i] = currentProcess.pid = rc;
            enqueue(&readyQueue[2], currentProcess);
        }
    }

    sleep(5);

    /* Setting the handler and the timer */
    struct sigaction sact;
    struct itimerval itimer;
    memset(&sact, 0, sizeof(sact));
    sact.sa_handler = scheduler;
    sigaction(SIGALRM, &sact, NULL);
    itimer.it_interval.tv_sec = 1;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec = 0;
    itimer.it_value.tv_usec = 200;
    setitimer(ITIMER_REAL, &itimer, NULL);

    /* Scheduler Ready
     * Wait for the scheduler until the end of the running time
     * then terminate all the child processes */
    while(alarmCount <= runningTime);
    for(int i = 0; i < processNum; i++) {
        kill(pidList[i], SIGINT);
    }
    return 0;
}
