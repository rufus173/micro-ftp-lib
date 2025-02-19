#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "../lib/mftplib.h"
void client_cleanup(){
	printf("client stopping.\n");
}
int client_main(int argc, char **argv){
	printf("Starting client...\n");
	atexit(client_cleanup); //cleanup for when exit() is called

	struct mftp_connection *connection = mftp_connect("192.168.1.156",NULL);
	if (connection == NULL){
		perror("mftp_connection");
		exit(EXIT_FAILURE);
	}
	printf("connected\n");

	for (int i = 0; i < 5; i++){
		struct mftp_communication_chunk chunk;
		snprintf(chunk.data,MAX_CHUNK_DATA_SIZE,"hello world for the %d time",i);
		int result = mftp_send_communication_chunk(connection,&chunk,0);
		if (result < 0) break;
	}

	mftp_disconnect(connection);
}
