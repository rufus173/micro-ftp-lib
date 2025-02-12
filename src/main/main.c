#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../lib/mftplib.h"
int main(int argc, char **argv){
	struct mftp_connection *connection = mftp_connect("0.0.0.0",NULL);
	if (connection == NULL){
		fprintf(stderr,"failed to connect.\n");
		return EXIT_FAILURE;
	}


	mftp_disconnect(connection);
}
