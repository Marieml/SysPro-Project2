/*
	Name Maria Miliou
	A.M.:1115201300101
*/

#include<stdlib.h>
#include<stdbool.h>
#include"linked_list.h"

/*-----Implementation of queue using linked list structure-----*/

//Initialization
List list_init(){
	List l = malloc(sizeof(List));
	l->list_head = NULL;
	l->list_tail = NULL;
	l->items = 0;
	return l;
}

//Push item returns true if succeed otherwise false
bool list_add(List l,item_t it)
{
	struct node *newNode = malloc(sizeof(struct node));

	if(newNode==NULL)
		return false;

	newNode->item=it;
	newNode->next = NULL;

	if(l->list_tail!=NULL)
		l->list_tail->next=newNode;

	l->list_tail=newNode;

	if(l->list_head==NULL) //First insert
		l->list_head=l->list_tail;

	l->items++;

	return true;
}

//Pop item and return
item_t list_remove(List l)
{
	struct node *temp;
	item_t item;

	item = l->list_head->item;
	temp = l->list_head->next;

	free(l->list_head);
	l->list_head=temp;

	l->items--;
	return item;
}

//Returns true id queue is empty, otherwise false
bool list_isempty(List l)
{
	return (l->list_head==NULL);
}

//Custom find
//Search socket_mutex by socket_id
pthread_mutex_t* list_find(List l, int s_id){

	struct node *temp = l->list_head;
	while(temp!=NULL){
		if(temp->item.socket ==s_id )
			return temp->item.p_mutex;
		temp=temp->next;
	}
}

//Destroy queue
void list_destroy(List l){
	while(!list_isempty(l)){
		list_remove(l);
	}
	free(l);
}