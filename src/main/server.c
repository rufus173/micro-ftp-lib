#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "../lib/mftplib.h"
int server_main(int argc, char **argv){
	printf("Starting server...\n");
	atexit(server_cleanup); //cleanup for when exit() is called
	
	//====== start listening for connections ======
	printf("listening for connections\n");
	struct mftp_connection *connection = mftp_listen(NULL);
	if (connection == NULL){
		perror("mftp_listen");
		exit(EXIT_FAILURE);
	}

	printf("connection found.\n");

	for (;;){
		struct mftp_communication_chunk chunk;
		int result = mftp_recv_communication_chunk(connection,&chunk,NULL,NULL,0);
		if (result < 0){
			break;
		}
		printf("%s\n",chunk.data);
	}

	mftp_disconnect(connection);
}
void server_cleanup(){
	printf("Server terminating\n");
}
