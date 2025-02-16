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
	mftp_disconnect(connection);

}
