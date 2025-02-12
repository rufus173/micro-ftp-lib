#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/mftplib.h"
int main(int argc, char **argv){
	struct mftp_connection *connection = mftp_connect("192.168.1.156",NULL);
	if (connection == NULL){
		fprintf(stderr,"failed to connect.\n");
		return EXIT_FAILURE;
	}

	struct mftp_communication_chunk *chunk = malloc(sizeof(struct mftp_communication_chunk));
	strcpy(chunk->data,"hello");

	mftp_send_communication_chunk(connection,chunk);
	free(chunk);

	free(mftp_recv_communication_chunk(connection));

	mftp_disconnect(connection);
}
