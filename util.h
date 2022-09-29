/*
	Name: Maria Miliou
	A.M.: 1115201300101
*/

#ifndef UTIL_H
#define UTIL_H

void perror_exit(char *msg);
void sanitize(char *str);
int write_data(int, char*);
int read_data(int, char*);

int myfind(char*, FILE *);

void *com_thread(void *arg);

void *worker_thread();
#endif