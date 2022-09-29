all:	dataServer \
		remoteClient

dataServer: dataServer.c queue.c linked_list.c util.c
	gcc dataServer.c queue.c linked_list.c util.c -o dataServer -lpthread

remoteClient: remoteClient.c
	gcc remoteClient.c -o remoteClient

clean:
	rm -rf dataServer
	rm -rf remoteClient