#include "ku_pfred.h"

int main(int argc, char* argv[]){

    int num_of_pr, interval, fd;
    int count, arr_cnt, remainder, start_pnt, temp, prio;
    int i, j;
    int* arr;
    int* read_cnt;
    char buf[32];
    struct mq_attr attr;
    mqd_t mqdes;

    // Decision of argc and argv
    if(argc != 4){
        printf("[ERROR]{ARGC} argc must be 4! \"./ku_pfred <number of process> <interval> <input file>\"\n");
        return -1;
    }

    else if((num_of_pr = atoi(argv[1])) <= 0 ||  num_of_pr > 17){
        printf("[ERROR]{ARGV} Number of process must be 0 < && <= 16\n");
        return -1;
    }

    else if((interval = atoi(argv[2])) <= 0 || interval > 10000){
        printf("[ERROR]{ARGV} Interval must be 0 < && <= 10000\n");
        return -1;
    }

    else if((fd = open(argv[3], O_RDONLY, 0444)) < 0){
        printf("[ERROR]{ARGV} Not invalid file: %s\n", argv[3]);
        return -1;
    }

#if MODE == 1
    printf("[]{PARENT} Argument Valid!\n");
    printf("[]{PARENT} i = %d\n", i);
    printf("[]{PARENT} buf = %s\n", buf);
#endif

    // READ COUNT
    start_pnt = 0;
    do{
        pread(fd, buf + start_pnt, 1, start_pnt);
    }while(buf[start_pnt++] != '\n');
    
    count = atoi(buf);
    memset(buf, 0, 32);

#if MODE == 1
    printf("[]{PARENT} count = %d\n", count);
#endif

    arr_cnt = 10000 / interval;
    arr_cnt += (10000 % interval) ? 1 : 0;

    arr = (int *)malloc(sizeof(int) * arr_cnt);
    read_cnt = (int *)malloc(sizeof(int) * num_of_pr);

    remainder = count % num_of_pr;

    for(i = 0; i < num_of_pr; i++)
        read_cnt[i] = count / num_of_pr;

    for(i = 0; i < remainder;)
        read_cnt[i++]++;

    // SETTING MESSAGE QUEUE
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;

    if((mqdes = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr)) == -1){
        perror("[ERROR]{PARENT} mq_open() failed!");
        exit(-1);
    }

    // FORK
    for(i = 0; i < num_of_pr; i++){
        if(fork() == 0){
            /* CHILD */
            for(j = 0; j < i; j++) // CALCULATE EACH PROCESSE'S START POINT
                start_pnt += (read_cnt[j] * 5); 

            for(j = 0; j < read_cnt[i]; j++){ // READ FILE AND ADD COUNT
                pread(fd, buf, 4, start_pnt + j * 5); 
#if MODE == 1
                printf("[READ]{CHILD #%d} buf = %s\n", i, buf);
#endif
                arr[atoi(buf) / interval]++;
            }

            for(j = 0; j < arr_cnt; j++){ // SEND ALL DATA TO PARENT
                if(mq_send(mqdes, (char *)&arr[j], MSG_SIZE, j) == -1)
                    perror("[ERROR]{CHILD} mq_send()");
#if MODE == 1
                else
                    printf("[SEND]{CHILD #%d} arr[%d] = %d\n", i, j, arr[j]);
#endif
            }
        
            mq_close(mqdes);
            free(read_cnt);
            free(arr);
            close(fd);

            exit(i);
        }
    }

    /* PARENT */
    j = 0;
    while(j++ < num_of_pr * arr_cnt){ // RECEIVE ALL DATA FROM CHILDREN
        if(mq_receive(mqdes, (char*)&temp, MSG_SIZE, &prio) == -1)
            perror("[ERROR]{PARENT} mq_receive()");
#if MODE == 1
        else
            printf("[RECV]{PARENT} arr[%d] = %d\n", prio, temp);
#endif
        arr[prio] += temp;
    }

    for(i = 0; i < num_of_pr; i++){ // REAPING CHILDREN
        waitpid(-1, &temp, 0);
#if MODE == 1
        if (WIFEXITED(temp))
            printf("[REAP]{PARENT} CHILD %d Terminated\n", WEXITSTATUS(temp));
        else
            printf("[SIGN]{PARENT} CHILD SEND SIGNAL %d\n", WIFSIGNALED(temp));
#endif
    }
        

#if MODE == 1
    printf("\n[]{PARENT} RESULT!\n\n");
#endif
    
    for(i = 0; i < arr_cnt; i++)
        printf("%d\n", arr[i]);

    // TERMINATE
    mq_close(mqdes);
    mq_unlink(MQ_NAME);
    free(read_cnt);
    free(arr);
    close(fd);

    return 0;
}
