
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
	connection->host_address_info = NULL;
	connection->connection_address_info = NULL;

	//======== fill in the neccesary paperwork for connecting =======
	struct addrinfo hints, *address_info, *host_address_info;
	memset(&hints,0,sizeof(struct addrinfo));

	//set the hints
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE; //for the bind getaddrinfo

	//get the address info for client
	int result = getaddrinfo(address,port,&hints,&address_info);
	if (result < 0){
		DEBUG_EXTRA fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		free(connection);
		return NULL;
	}
	connection->connection_address_info = address_info;
	//get the address info for this device
	result = getaddrinfo(NULL,port,&hints,&host_address_info);
	if (result < 0){
		DEBUG_EXTRA fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		free(connection);
		return NULL;
	}
	connection->host_address_info = host_address_info;

	//======= create a socket =======
	int sockfd = socket(address_info->ai_family,address_info->ai_socktype,0);
	connection->socket = sockfd;
	if (sockfd < 0){
		DEBUG_EXTRA perror("socket");
		mftp_disconnect(connection);
		return NULL;
	}

	/*
	//====== connect to set default sender address ======
	result = connect(sockfd,address_info->ai_addr,address_info->ai_addrlen);
	if (result < 0){
		DEBUG_EXTRA perror("connect");
		mftp_disconnect(connection);
		return NULL;
	}
	*/

	return connection;
}
int mftp_disconnect(struct mftp_connection *connection){
	DEBUG_EXTRA printf("disconnecting from connection.\n");
	//clean up allocated memory
	freeaddrinfo(connection->connection_address_info);
	freeaddrinfo(connection->host_address_info);
	free(connection);
}
int mftp_send_communication_chunk(struct mftp_connection *connection, struct mftp_communication_chunk *chunk){
	//unpack the struct a little
	int sockfd = connection->socket;

	//send untill the receiver gets it
	for (;;){
		int result = sendto(sockfd,chunk,sizeof(struct mftp_communication_chunk),MSG_CONFIRM,connection->connection_address_info->ai_addr,connection->connection_address_info->ai_addrlen);
		if (result < 0){
			DEBUG_EXTRA perror("send");
			return -1;
		}
		DEBUG_EXTRA printf("%d bytes out of %lu sent\n",result,sizeof(struct mftp_communication_chunk));
		break;
	}
}
struct mftp_communication_chunk *mftp_recv_communication_chunk(struct mftp_connection *connection){
	int sockfd = connection->socket;

	//to receive into
	struct mftp_communication_chunk *chunk = malloc(sizeof(struct mftp_communication_chunk));
	if (chunk == NULL){
		DEBUG_EXTRA perror("malloc");
		return NULL;
	}

	for (;;){
		int result = recvfrom(sockfd,&chunk,sizeof(struct mftp_communication_chunk),0,connection->connection_address_info->ai_addr,&(connection->connection_address_info->ai_addrlen));
		if (result < 0){
			DEBUG_EXTRA perror("recvfrom");
			free(chunk);
			return NULL;
		}
		break;
	}
	return chunk;
}
