Name: Maria Miliou
A.M.: 1115201300101
Course: K24 System Programming
		 ----------
		|PROJECT 2 |
		 ----------

----BASICS----------------------------------------------------------------------------------

- Submitted files:

	* Server's code
	- dataServer.c		- Makefile
	- queue.h			- util.h
	- queue.c			- linked_list.h
	- linked_list.c		
	- util.c

	* Client's code
	- remoteClient.c

- Compilation: 	* dataServer: type <make dataServer>
				* remoteClient: - 1st way: Copy Makefile and place it in the same directory with
										    remoteClient.c and type <make remoteClient> for compilation.
								- 2nd way: gcc remoteClient.c -o remoteClient

- Execution: * Local mashine: Run ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>
			 * Local or remote mashine: ./remoteClient -i <server_ip> -p <server_port> -d <directory>

			 Note: * To find out server's IP run <hostname -I> (server's host)
			 	   * Maximum size of file to copy 128 bytes

- System: I run my program in Linux DIT, specially linuxYY.di.uoa.gr (where YY = 02, 14, 15).


	  	  		

----PROGRAM-------------------------------------------------------------------------------------

-- Server (dataServer.c)

- Global
	* Define a queue
	* Define a list (linked list) to store mutexes for each socket-client
	  to ensure that only one worker writes to socket

- main()
	* Initialize Queue and list of mutexes 
	* Creates a TCP connection
	* Initialize mutex and condition variables(empty, full) for accessing to queue
	* Create <thread_pool_size> worker threads
	* Begin infinity loop
	  - Accept connection
	  - Creates a communication thread
___________________________________________________________________

-- Client (remoteClient.c)

- main()
	* Get socket and connect
	* Write directory for copy to socket using write_all() to ensure that all bytes are written.
	* Reads repeatedly from socket file's name as path that server send.
	  - Finds from file's path directory/ies using strtok and creates them if not exist.
	  - Creates new file. If file exist, delete it and recreate.
	  - Reads from socket metadata (file size).
	  - Reads from socket file size bytes and writes it to new created file (This is context of file)
	  
- read_data(int, char*)
	* This function is from Topic 5 of lecture --> program <prsref.c>
	* Use: Reads files (as path) from socket.
	* Arguments: file descriptor of socket and buffer.
	* Reads from socket formated data.
	* First read length of string that server is sending
	* Then read length bytes from socket

- write_all(int, void*, size_t)
	* This function is from Topic 5 of lecture --> program <rls.c>
	* Use: Writes directory for copy to socket and ensure that all bytes are written.
	* Arguments: file descriptor of socket, buffer, bytes to write.

- perror_exit(char *)
	* This function is from Topic 5 of lecture --> program <rls.c>
	* perror combined with exit

_________________________________________________________________________

-- <util.c>
Various functions used by dataServer and thread functions

-  sanitize(char *)
	* This function is from Topic 5 of lecture --> program <rlsd.c>
	* To ensure that client not passing a dirname like * "; rm *"

- perror_exit(char *)
	* See Client

- read_data(int, char*)
	* See Client

- write_data(int, char*) 
	* This function is from Topic 5 of lecture --> program <prsref.c>
	* Use: Writes files (as path) to socket.
	* Arguments: file descriptor of socket and buffer.
	* Writes to socket length of file's path first, and then file's path 

- myfind(char *, FILE *)
	* Reads recursively a directory and writes to stream fp
    	  names of regular files seperated with \n.
	* Used by communication thread 

// Communication thread
- com_thread(void *)
	* Argument: socket's file descriptor
	* Read directory from socket
	* Creates a new file with unique name <thread_id> for read/write files in directory
	* Call myfind which writes to new file all regular files in directory recursively
	* Reads from this file all regular files that myfind found and push them to queue 
	* Deletes created file 

// Worker thread
- worker_thread(void *)
	* Arguments: block_size
	* Begin an infinity loop
	  - Pops from queue information (<file's name, socket>)
	  - Lock mutex to socket
	  - Writes file name to socket using write_data()
	  - Writes file size and his context

_______________________________________________________________________

-- <linked_list.c> 

  	* Data structure linked list to store mutexes to access a socket
	* Stores items struct entry{int socket, phtread_mutex_t * }
	* Operations: init, add, remove, isempty, destroy, find
 	* The first 5 is a basic implemention of these operations like I learned from course and book of Data Structures.
	* list_find() : By given socket file descriptor returns its pointer to mutex

_______________________________________________________________________________

-- <queue.c>

	* queue_init: Initialize queue
	* queue_isFull: Return True or False if queue is full
	* queue_isempty: Return True or False if queue is empty
	* queue_destroy: Destroy queue (free memory)
	* queue_push:
	  - Lock queue mutex to access queue
	  - While it is full release and wait (using pthread_cond_wait)
	  - When an item is removed from queue and there is some space, push new item
	    to queue and unlock mutex.
	* queue_pop:
	  - Lock queue mutex to access queue
	  - While it is empty release mutex and wait (using pthread_cond_wait)
	  - When an item is placed to queue, pop it from queue and unlock mutex.

-- Libraries

	- queue.h
	* Definition of queue's functions in queue.c

	- util.h
	* Definition of various utilities (functions) as long as thread functions: communication
	  and worker thread.

	- linked_list.h
	* Definition of linked list's structures and functions in linked_list.c


