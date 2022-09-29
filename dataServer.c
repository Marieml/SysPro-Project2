/* 
Name: Maria Miliou
A.M.: 1115101300101
Course: System Programing

---------------------------------------- Project 2---------------------------------------
*/

#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netdb.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "util.h"
#include "queue.h"
#include "linked_list.h"
#define perror2(s,e) fprintf(stdout, "%s: %s\n", s, strerror(e))


queue_t Queue;

//Linked list of mutexes --> one worker is writing to client at a time
List list_mutex;

int main(int argc, char *argv[]) {
    int	lsock, csock;
    int err, status;
    int port, thread_pool_size, queue_size, block_size;
    pthread_t thr;
    item_t added;
    const int opt = 1;
    
    unsigned int clientlen;
    struct sockaddr_in myaddr, client;
    struct stat statbuf;
    struct hostent *cl;



    //Arrguments checking
    if(argc != 9){
        printf("Wrong arguments! Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n");
        exit(EXIT_FAILURE);
    }
    printf("Server's parameters are:\n");
    for(int i=1 ; i<9; i=i+2){
        if(strcmp(argv[i], "-p") == 0){
            port = atoi(argv[i+1]);
            printf("port: %d\n", port);
        }else if(strcmp(argv[i], "-s") == 0){
            thread_pool_size = atoi(argv[i+1]);
            printf("thread_pool_size: %d\n", thread_pool_size);
        }else if(strcmp(argv[i], "-q") == 0){
            queue_size = atoi(argv[i+1]);
            printf("queue_size: %d\n", queue_size);
        }else if(strcmp(argv[i], "-b") == 0){
            block_size = atoi(argv[i+1]);
            printf("Block_size: %d\n", block_size);
        }else{ 
            printf("Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n");
            exit(EXIT_FAILURE);
        }
    }
    
    //Initialize Queue
    queue_init(&Queue, queue_size);

    //Initialize list of mutexes for socket
    list_mutex=list_init();

    //----SOCKETS-------//

    // Create a TCP connection
    if ((lsock = socket( AF_INET, SOCK_STREAM, 0)) < 0)
    	perror_exit( "socket" );

    //Enable reuse of address
    if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0)
        perror_exit("setsockopt");

    // Bind address to socket
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(port);/*port to bind socket*/
    myaddr.sin_family = AF_INET;  /* internet addr family */
    if(bind(lsock,(struct sockaddr *)&myaddr, sizeof(myaddr)))
        perror_exit( "bind" );

    printf("Server was successfully initialized...\n");

    // Listening for connections Qsize=5 
    if(listen(lsock, 5) != 0 )
    	perror_exit( "listen" );
    printf("Listening for connections to port %d\n", port);

    //-------THREADS---------//

    pthread_mutex_init(&queue_mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);

    pthread_t *workerids;

    // Create <thread_pool_size> worker threads 
    if((workerids = malloc(thread_pool_size*sizeof(pthread_t))) == NULL)
        perror_exit("malloc");

    for(int i=0; i<thread_pool_size; i++){
        if(err = pthread_create(workerids+i, NULL, worker_thread, (void *) block_size)){
            perror2("pthread_create",err);
            exit(1);
        }

        if(err = pthread_detach(*(workerids+i))){
            perror2("pthread_detach",err);
            exit(1);
        }
    }

    // Main loop: accept, create new communication thread
    while(1){ 
            clientlen = sizeof(client);
            // Accept connection
            if ( (csock = accept(lsock, (struct sockaddr *)&client, &clientlen)) < 0 )
    	       perror_exit("accept");

            // Find client's name
            if((cl = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL)
                perror_exit("gethostbyaddr");

            printf ( "Accepted connection from %s\n", cl->h_name);

            //Create a mutex for this socket and add it to list
            pthread_mutex_t socket_mtx;
            pthread_mutex_init(&socket_mtx, 0);

            //Add it list
            added.socket = csock;
            added.p_mutex = &socket_mtx;

            pthread_mutex_lock(&socket_mtx);
            list_add(list_mutex, added);
            pthread_mutex_unlock(&socket_mtx);

            //Create thread
            if(err=pthread_create(&thr, NULL, com_thread, (void *) csock)){
                perror2("pthread_create", err);
                exit(1);
            }
    }
    queue_destroy(&Queue);
    list_destroy(list_mutex);
}