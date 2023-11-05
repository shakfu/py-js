#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* worker_function(void* arg);
pthread_mutex_t work_mutex; // our little flag

int main(int argc, char* argv[])
{
    int res;
    pthread_t worker_thread;
    void* thread_result;
    res = pthread_mutex_init(&work_mutex, NULL);
    if (res != 0) {
        perror("Mutex Initialization Failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_create(&worker_thread, NULL, worker_function, NULL);
    if (res != 0) {
        perror("Thread creation Failed");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < 10; i++) {
        pthread_mutex_lock(&work_mutex);
        printf("Main Thread: %d\n", i);
        pthread_mutex_unlock(&work_mutex);
    }

    printf("Waiting for worker thread to finish\n");
    res = pthread_join(worker_thread, &thread_result);
    if (res != 0) {
        perror("Worker thread crashed");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_destroy(&work_mutex);
    exit(EXIT_SUCCESS);
}

void* worker_function(void* arg)
{
    int i;
    for (i = 0; i < 20; i++) {
        pthread_mutex_lock(&work_mutex);
        printf("Worker thread: %d\n", i);
        pthread_mutex_unlock(&work_mutex);
    }
    pthread_exit(0);
}
