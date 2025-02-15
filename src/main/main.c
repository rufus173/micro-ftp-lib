#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/mftplib.h"
int main(int argc, char **argv){
	struct mftp_connection *connection = mftp_connect(NULL,NULL);
	if (connection == NULL){
		fprintf(stderr,"failed to connect.\n");
		return EXIT_FAILURE;
	}

	struct mftp_communication_chunk *chunk = malloc(sizeof(struct mftp_communication_chunk));
	strcpy(chunk->data,"hello");

	for (;;){
		int result = mftp_send_communication_chunk(connection,chunk);
		if (result < 0){
			fprintf(stderr,"could not send data");
			perror("send_com_chunk");
			free(chunk);
			mftp_disconnect(connection);
			exit(EXIT_FAILURE);
		}
	}
	free(chunk);

	//free(mftp_recv_communication_chunk(connection));

	mftp_disconnect(connection);
}
