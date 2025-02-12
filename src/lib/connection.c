
#include "definitions.h"

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct mftp_connection *mftp_connect(char *address, char *port){
	if (port == NULL){
		port = DEFAULT_MFTP_PORT;
	}
	DEBUG_EXTRA printf("setting up connection to %s:%s\n",address,port);
	
	//prepare the return struct
	struct mftp_connection *connection;
	connection = malloc(sizeof(struct mftp_connection));
	if (connection == NULL){
		DEBUG_EXTRA perror("malloc");
		return NULL;
	}
	memset(connection,0,sizeof(struct mftp_connection));

	//======== fill in the neccesary paperwork for connecting =======
	struct addrinfo hints, *address_info;
	memset(&hints,0,sizeof(struct addrinfo));

	//set the hints
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_UNSPEC;

	//get the address info
	int result = getaddrinfo(address,port,&hints,&address_info);
	if (result < 0){
		DEBUG_EXTRA fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		free(connection);
		return NULL;
	}
	connection->address_info = address_info;

	//======= create a socket =======
	int sockfd = socket(address_info->ai_family,address_info->ai_socktype,0);
	connection->socket = sockfd;
	if (sockfd < 0){
		DEBUG_EXTRA perror("socket");
		mftp_disconnect(connection);
		return NULL;
	}

	return connection;
}
int mftp_disconnect(struct mftp_connection *connection){
	//clean up allocated memory
	freeaddrinfo(connection->address_info);
	free(connection);
}
