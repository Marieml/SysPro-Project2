/*
	Name: Maria Miliou
	A.M.: 1115201300101
*/

#ifndef QUEUE_H
#define	QUEUE_H

#include <stdbool.h>
#include <pthread.h>


//Mutex and conditional variables for accessing queue (init)
pthread_mutex_t queue_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

struct data{
	int socket;
	char filename[256];
};

typedef struct{
    struct data *data_p;
    int start;
    int end;
    int count;
    int maxsize;
} queue_t;

void queue_init(queue_t *, int);
bool queue_isFull(queue_t *);
bool queue_isempty(queue_t *);
void queue_destroy(queue_t *);
void queue_push(queue_t *, struct data);
void queue_pop(queue_t *, struct data*);

#endif
