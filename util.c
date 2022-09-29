#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "util.h"
#include "queue.h"
#include "linked_list.h"


extern List list_mutex;
extern queue_t Queue;

// From lecture Topic 5 rlsd.c
void sanitize(char *str)
{
	char *src, *dest;
	for ( src = dest = str ; *src ; src++ )
		if ( *src == '/' || isalnum(*src) )
			*dest++ = *src;
	*dest = '\0';
}

// Print error message and exit
void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int read_data (int fd, char *buffer){/* Read formated data */
	char temp;int i = 0, length = 0;
	if ( read ( fd, &temp, 1 ) < 0 )	/* Get length of string */
		exit (-3);
	length = temp;
	while ( i < length )	/* Read $length chars */
		if ( i < ( i+= read (fd, &buffer[i], length - i)))
			exit (-3);
	return i;	/* Return size of string */
}

int write_data ( int fd, char* message ){/* Write formated data */
	char temp; int length = 0;
	length = strlen(message) + 1;	/* Find length of string */
	temp = length;
	if( write (fd, &temp, 1) < 0 )	/* Send length first */
		exit (-2);
	if( write (fd, message, length) < 0 )	/* Send string */
		exit (-2);
	return length;		/* Return size of string */
}

/*
    Reads recursively a directory and writes to stream fp
    names of regular files seperated with \n.
*/
int myfind(char *dirname, FILE *fp){

    DIR *dp;
    int count=0;
    struct dirent *dir;
    struct stat info;
    char *newname;

    //Open directory
    if((dp=opendir(dirname)) == NULL)
        perror("opendir");

    //While reading files in directory
    while((dir = readdir(dp)) != NULL){
        if(dir->d_ino == 0)
            continue;
        
        //stat() to find type of path+filename
        newname=(char *)malloc(strlen(dirname)+strlen(dir->d_name)+2);
        strcpy(newname, dirname);
        strcat(newname,"/");
        strcat(newname, dir->d_name);
        
        if(stat(newname, &info) <0){
            perror("stat");
        }

        //If directory
        if((info.st_mode & S_IFMT) == S_IFDIR){
            //Do not consider . and  . . directories
            if(memcmp(dir->d_name, ".", 1) == 0 || memcmp(dir->d_name, ". .", 3) == 0){
               free(newname);
               continue;
            }
            //Call recursively myfind for this directory
            count += myfind(newname, fp);
        }

        //If regular file write to file
        if((info.st_mode & S_IFMT) == S_IFREG){
            if(fwrite(newname, 1, strlen(newname), fp)<0)
                perror_exit("write");
            fwrite("\n",1 , 1, fp);
            count++;        
        }
        free(newname);
    }
    closedir(dp);
    return count;
}

//------------------- Communication thread ------------------------------//
// Producer
void *com_thread(void *arg){    
    char    dirname[BUFSIZ];    // Directory from client
    FILE *fp;
    struct data pushed;
    int csock = (int) arg;
    char threadfile[256];
    int count;      //Total files in directory
    char num[10];

    // Read directory from socket with client
    if(read(csock, dirname, BUFSIZ) <0 )
    	perror_exit("reading dirname");
    // Clear wild characters
    sanitize(dirname);
    
    printf("[Thread: %ld]: About to scan directory %s\n ",pthread_self() , dirname);
    
    //Create new file with unique name <thread_id> for read/write files in directory
    memset(threadfile, 0, 256);
    sprintf(threadfile, "%ld", pthread_self());
    if((fp = fopen(threadfile,"w+")) == NULL)
        perror_exit("fopen");
    
    //Find all regular files and write to threadfile
    count = myfind(dirname,fp);

    //Write to socket count of files so client knows when to stop
    sprintf(num, "%d", count);
    write(csock, num, strlen(num));
           
    //Set pointer to beginning
    fseek(fp, 0,SEEK_SET);

    // Read 1 filename at a time from threadfile
    while(fgets(dirname, BUFSIZ, fp) != NULL ){

        pushed.socket = csock;

        //Remove path to directory fro copy
        strncpy(pushed.filename, dirname, strlen(dirname)-1);  // Watch out dirname + \n -----> dirname[size-1]
        printf("[Thread: %ld]: Adding file %s to the queue...\n ",pthread_self() , pushed.filename);

        queue_push(&Queue, pushed);

        // Queue is not empty. Send signal.
        pthread_cond_signal(&cond_nonempty);

	memset(dirname, 0, BUFSIZ);
        memset(pushed.filename, 0, 256);

    }
        
    fclose(fp);
    if(unlink(threadfile)<0)
        perror_exit("unlink");

    pthread_exit(NULL);
}

//------------------- Worker thread ------------------------------//

// Consumer
void *worker_thread(void *arg){
    int	c, fd, size, nread;
    struct data poped;
    struct stat statbuf;
    char temp; //Sending file size
    pthread_mutex_t *sock_mtx;

    int block_size = (int) arg;
    char    buffer[block_size]; //512 block size

    while(1){
        queue_pop(&Queue, &poped);

        printf("[Thread: %ld]: Received task: <%s, %d>\n", pthread_self(), poped.filename, poped.socket);

        // Queue is not full. Send signal.
        pthread_cond_signal(&cond_nonfull);

        //Find and lock socket mutex
        sock_mtx=list_find(list_mutex, poped.socket);
        pthread_mutex_lock(sock_mtx);

        //Write file name to socket 
        write_data(poped.socket, poped.filename); 

        //Open file for reading
        if((fd = open(poped.filename, O_RDONLY))<0)
            perror_exit("open");
       
        //Write file size
        if(fstat(fd, &statbuf))
            perror("fstat");
        size=statbuf.st_size;
        temp=size;
        if(write(poped.socket, &temp, 1)<0)
            perror_exit("write");

        //Write file context            
        printf("[Thread: %ld]: About to read file: %s\n", pthread_self(), poped.filename);
        while((nread = read(fd, buffer, block_size))>0){
                if(write(poped.socket, buffer, nread)<0){
                    perror_exit("write");
                }
        }
        if(nread <0)
            perror("read");

        close(fd);
        pthread_mutex_unlock(sock_mtx);
    }
    pthread_exit(NULL);
}