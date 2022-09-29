# Project 2

# Goal
The purpose of this assignment is to get familiar with the following:
* Threads 
* Inter-process communication

This program copies recursively the content of a server's directory to local client's file system.

Server can recieve requests from multiple clients.

Client sends request for desired directory he wants to copy from sever (we asume that client knows that this directory exist on server). Server sends the information about this directory and client creates a local copy of this. The copied directory must have the same structure and files as the original. 

Communication between server and client is accomplished using sockets.

## Server
* Creates a new **communication thread** for each new accepted connection. This type of thread is responsible to read the name of the directory asked from client. Server reads the directory and place all files and subdirectories to a queue (only names not file's content). If queue is full, **communication thread** must wait until there is empty space available.

* Server also has a thread pool with **worker threads**. The pool size passes from command line argument. Each worker thread is responsible to read the content of a file and send it to client through socket. The file will be readed and sent per block. The size of block passes from command line argument (bytes).
## Submitted files:

* Server's code
	- dataServer.c		
	- Makefile
	- queue.h			
	- util.h
	- queue.c			
	- linked_list.h
	- linked_list.c		
	- util.c

* Client's code
    - remoteClient.c

## Compilation:
* dataServer: type 
```
make dataServer
```
* remoteClient: 
- 1st way: Copy Makefile and place it in the same directory with  remoteClient.c and type
```
make remoteClient>
```
- 2nd way: 
```
gcc remoteClient.c -o remoteClient
```

## Execution:
* Local mashine: Run
```
./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>
```
* Local or remote mashine:
```
./remoteClient -i <server_ip> -p <server_port> -d <directory>
```
Note: 
* To find out server's IP run <hostname -I> (server's host)
* Maximum size of file to copy 128 bytes

## System:
I run my program in Linux DIT, specially linuxYY.di.uoa.gr (where YY = 02, 14, 15).

More detailed explanation of code can be found in README.txt
	  	  		
