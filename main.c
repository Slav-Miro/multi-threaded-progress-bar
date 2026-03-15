#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define ESCAPE "\x1B[%d;%dH%s"
#define TASKS 20
#define COUNT 80

pthread_t thread_array[TASKS];
typedef struct data_struct{
    int task_id;
    int task_time;
    int row;
    int col;
    int count;
}data_struct;

pthread_mutex_t stdout_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

void * thread_handler(void * argument);

int main(void){
    printf("\033[2J\033[H");
    //fflush(stdout);
    //Malloc an array of tasks to pass to the thread
    data_struct * data_pointer = malloc(sizeof(data_struct) * TASKS);
    if (!data_pointer) { printf("Error in allocating data array\n"); }

    for (int i = 0 ; i < TASKS; i++){
        data_pointer[i].col = 15;
        data_pointer[i].row = i + 3;
        data_pointer[i].task_time =(rand() % 10) + 1;
        data_pointer[i].count = 0;

        //Create thread and assign it's respective data
        pthread_create(&thread_array[i], NULL, thread_handler, &data_pointer[i]);
    }

    //Join the threads
    for (int i = 0 ; i < TASKS; i++){
        pthread_join(thread_array[i], NULL);
    }

    free(data_pointer);

    printf("\033[%d;1H", TASKS + 2);
    return 0;
}

void * thread_handler(void * argument){
    data_struct * data = (data_struct *)argument;
    pthread_mutex_lock(&stdout_mutex_lock);
    data->task_id =  pthread_self() % 10000;
    printf("\033[%d;%dHTASK ID: %4d: ",data->row, 1,data->task_id);
    for (int i = 0 ; i < COUNT - 1; i++){
        printf("\033[38;5;226m-");
    }
    printf("\033[38;5;255m");
    pthread_mutex_unlock(&stdout_mutex_lock);

    //Since our shared data is the stdout we have to lock it and unlock it when printing
    while (data->count < COUNT){
        pthread_mutex_lock(&stdout_mutex_lock);
        printf("\033[38;5;46m"ESCAPE, data->row, data->col , "#");
        printf("\033[38;5;255m");
        data->col++;
        data->count++;
        fflush(stdout);
        data->task_time =(rand() % 10) + 1;
        if (data->task_time == 0) { data->task_time = 5; }
        pthread_mutex_unlock(&stdout_mutex_lock);
        usleep(10000 * data->task_time);
    }
    return NULL;
}