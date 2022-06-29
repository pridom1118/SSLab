#include "ku_tfred.h"

void* thread_function(void* data){
    /* CHILD */
    char buf[4];
    int i, j, th_pnt;

    pthread_mutex_lock(&mutex);
    i = rank++;
    pthread_mutex_unlock(&mutex);
    
    th_pnt = start_pnt;
    for(j = 0; j < i; j++)
        th_pnt += (read_cnt[j] * 5); 

    for(j = 0; j < read_cnt[i]; j++){
        pread(fd, buf, 4, th_pnt + j * 5); 
        pthread_mutex_lock(&mutex);
        arr[atoi(buf) / interval]++;
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char* argv[]){

    int num_of_th, count, arr_cnt, remainder;
    int i, j;
    char buf[256];

    pthread_t* tid;
    rank = 0;

    // Decision of argc and argv
    if(argc != 4){
        printf("[ERROR]{ARGC} argc must be 4! \"./ku_tfred <number of thread> <interval> <input file>\"\n");
        return -1;
    }

    else if((num_of_th = atoi(argv[1])) <= 0 || num_of_th > 17){
        printf("[ERROR]{ARGV} Number of thread must be 0 < && <= 16\n");
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

    // THREAD ARRAY
    tid = (pthread_t *)malloc(sizeof(pthread_t) * num_of_th);
    
    // READ COUNT
    start_pnt = 0;
    do{
        pread(fd, buf + start_pnt, 1, start_pnt);
    }while(buf[start_pnt++] != '\n');
    count = atoi(buf);

    arr_cnt = 10000 / interval;
    arr_cnt += (10000 % interval) ? 1 : 0;
    
    arr = (int *)malloc(sizeof(int) * arr_cnt);
    memset(arr, 0, sizeof(int) * arr_cnt);
    read_cnt = (int *)malloc(sizeof(int) * num_of_th);
    remainder = count % num_of_th;

    for(i = 0; i < num_of_th; i++)
        read_cnt[i] = count / num_of_th;

    for(i = 0; i < remainder;)
        read_cnt[i++]++;

    // FORK
    for(i = 0; i < num_of_th; i++)
        pthread_create(&tid[i], NULL, thread_function, NULL);

    for(i = 0; i < num_of_th; i++)
        pthread_join(tid[i], NULL);
    
    /* PARENT */
    for(i = 0; i < arr_cnt; i++)
        printf("%d\n", arr[i]);

    // TERMINATE
    free(tid);
    free(read_cnt);
    free(arr);
    close(fd);

    return 0;
}