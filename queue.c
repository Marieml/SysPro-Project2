/*
	Name Maria Miliou
	A.M.:1115201300101
*/

#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<pthread.h>
#include <string.h>
#include"queue.h"

//------Queue

void queue_init(queue_t *Queue, int N){
    Queue->data_p = malloc(sizeof(struct data)*N);
    Queue->start=0;
    Queue->end=-1;
    Queue->count=0;
    Queue->maxsize=N;
}

bool queue_isFull(queue_t *Queue){
    return (Queue->count >= Queue->maxsize);
}

bool queue_isempty(queue_t *Queue){
    return (Queue->count <=0);
}

void queue_destroy(queue_t *Queue){
    free(Queue->data_p);
}

void queue_push(queue_t *Queue, struct data d){
    // Lock queue mutex to access queue
    pthread_mutex_lock(&queue_mtx);
    // As long as queue is full ---> unlock and wait
    while( queue_isFull(Queue)){
        pthread_cond_wait(&cond_nonfull, &queue_mtx);
    }

    // Now there is some space. Pushing to queue filename and socket id.
    Queue->end = (Queue->end +1)% Queue->maxsize;
    Queue->data_p[Queue->end]=d;
    Queue->count++;

    //Unlock
    pthread_mutex_unlock(&queue_mtx);
}

void queue_pop(queue_t *Queue, struct data *dp){

    // Lock queue mutex to access queue
    pthread_mutex_lock(&queue_mtx);

    // As long as queue is empty ----> unlock and wait
    while(queue_isempty(Queue)){
        pthread_cond_wait(&cond_nonempty, &queue_mtx);
    }

    // Now there is an entry. Popping from queue filename and socket id.
    dp->socket = Queue->data_p[Queue->start].socket;
    strcpy(dp->filename, Queue->data_p[Queue->start].filename);
    Queue->start = (Queue->start + 1)%Queue->maxsize;
    Queue->count--;

    //Unlock
    pthread_mutex_unlock(&queue_mtx);      

}