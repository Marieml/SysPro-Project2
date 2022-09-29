/*
	Name: Maria Miliou
	A.M.: 1115201300101
*/

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

//Linked list defs
struct entry{
	int socket;
	pthread_mutex_t *p_mutex;
};

typedef struct entry item_t;

struct node
{
	item_t item;
	struct node *next;
};


struct linked_list{
	struct node *list_head;
	struct node *list_tail;
	int items;
};

//Linked list
typedef struct linked_list* List;

List list_init();
bool list_add(List,item_t);
item_t list_remove(List);
bool list_isempty(List);
void list_destroy(List);
pthread_mutex_t* list_find(List, int);

#endif