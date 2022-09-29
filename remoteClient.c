/*
Name: Maria Miliou
A.M.: 1115201300101
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <errno.h>
#define  BUFFSIZE 256

extern int errno;

void perror_exit(char *msg); 
int read_data(int, char*);
int write_all(int, void *, size_t);


int main(int argc, char *argv[]) {
    struct sockaddr_in  servadd; //Server address 
    struct hostent *hp;
    int    sock, fd, n_read;
    char   buffer[BUFFSIZE];
    struct in_addr myaddress ;

    int len;
    char temp;
    int port, pos_dir, pos_ip;
    int N; //Count of files to read
    

    // Check arguments
    if(argc != 7){
        printf("Wrong arguments! Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Client's parameters are:\n");
    for(int i=1 ; i<7; i = i+2){
        if(strcmp(argv[i], "-i") == 0){
            pos_ip = i+1;
            printf("serverIP: %s\n", argv[pos_ip]); //Server's IP in position pos_ip in argv
        }else if(strcmp(argv[i], "-p") == 0){
            port = atoi(argv[i+1]);             // Port
            printf("port: %d\n", port);
        }else if(strcmp(argv[i], "-d") == 0){
            pos_dir = i+1;               //Directory for copy in position pos_dir in argv
            printf("directory: %s\n", argv[pos_dir]);
        }else{ 
            printf("Usage: ./remoteClient -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("Connecting to %s on port %d\n", argv[pos_ip], port);

    // Network byte order
    inet_aton(argv[pos_ip] ,&myaddress);

    // Get socket
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit( "socket" );

    // Connect
    if((hp = gethostbyaddr((const char *)&myaddress, sizeof(myaddress), AF_INET)) == NULL) {
        herror("gethostbyaddr");
        exit(1);
    }

    memcpy(&servadd.sin_addr, hp->h_addr, hp->h_length);
    servadd.sin_port = htons(port);
    servadd.sin_family = AF_INET ;
    if(connect(sock, (struct sockaddr*) &servadd, sizeof(servadd)) !=0)
        perror_exit( "connect" );
    
    // Write directory and newline to socket 
    if ( write_all(sock, argv[pos_dir], strlen(argv[pos_dir])) == -1)
        perror_exit("write");
    if ( write_all(sock, "\n", 1) == -1 )
        perror_exit("write");


    //Find path to directory as bytes (act) to remove it while recreating later
    int act;
    char *token, *token1, *ptr1, *ptr;
    token1 = strtok_r(argv[pos_dir], "/", &ptr);
    while((token = strtok_r(NULL, "/", &ptr)) != NULL){
        act = act + strlen(token1) + 1;
        token1=token;
    }

    //Read count of files about to recieve
    read(sock, buffer, BUFFSIZE);
    N = atoi(buffer);

    // Read from socket file's name as path
    while((n_read = read_data(sock, buffer))>0){

        printf("Received: %s\n", &buffer[act]);
        //Find directories from path
        char *token, *token1, *ptr1;
        token = strtok_r(&buffer[act],"/", &ptr1);
        while((token1 = strtok_r(NULL, "/", &ptr1)) != NULL){    //As long as token is directory
                if(mkdir(token, 0777)<0 && errno != EEXIST) //Make directory. If exist ignore message
                    perror_exit("mkdir");
                sprintf(token, "%s%c%s", token, '/', token1);
        }

        //Create new file "token" 
            if((fd=open(&buffer[act], O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0777))<0){
                if (errno == EEXIST){   //If file already exist
                    if(unlink(buffer)<0) // Delete it
                        perror_exit("unlink");
                    if((fd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0777))<0) // Recreate
                        perror_exit("reopen");
                } else
                    perror_exit("open");
            }
    
        //Read metadata (file size)
        read(sock, &temp, 1);
        len = temp;

        // Read file context len bytes
        while(len > 0){
            n_read = read(sock, buffer, len);
            write(fd, buffer, n_read);
            len = len - n_read;
        }

        close(fd);

        N--;
        //If all files is send terminate
        if(!N)
            break;
    }
    close(sock);
    return 0;
}


//These functions is from lecture Topic 5 (rls.c and prsref)
int read_data (int fd, char *buffer){// Read formated data
	char temp;
    int i = 0, length = 0;
	if (read(fd, &temp, 1) < 0)	// Get length of string
		exit (-1);
	length = temp;
	while (i < length)
		if (i < ( i+= read (fd, &buffer[i], length - i)))
			exit(-1);
	return i;	//Return size of string
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

//Write until all bytes are written
int write_all(int fd, void *buff, size_t size) {
    int sent, n;
    for(sent = 0; sent < size; sent+=n) {
        if ((n = write(fd, buff+sent, size-sent)) == -1) 
            return -1;
    }
    return sent;
}