
#include "definitions.h"

#include <poll.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <time.h>

struct mftp_connection *mftp_create_connection(char *port){
	if (port == NULL){
		port = DEFAULT_MFTP_PORT;
	}
	DEBUG_EXTRA printf("setting up connection with port %s\n",port);
	
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
	connection->connection_addr = NULL;
	connection->previous_timestamps_oldest = 0;
	memset(connection->previous_timestamps,0,sizeof(connection->previous_timestamps));

	//======== fill in the neccesary paperwork for connecting =======
	struct addrinfo hints, *host_address_info;
	memset(&hints,0,sizeof(struct addrinfo));

	//set the hints
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE; //for the bind getaddrinfo

	//get the address info for this device
	int result = getaddrinfo(NULL,port,&hints,&host_address_info);
	if (result < 0){
		DEBUG_EXTRA fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		free(connection);
		return NULL;
	}
	connection->host_address_info = host_address_info;

	//======= create a socket =======
	int sockfd = socket(host_address_info->ai_family,host_address_info->ai_socktype,0);
	connection->socket = sockfd;
	if (sockfd < 0){
		DEBUG_EXTRA perror("socket");
		mftp_disconnect(connection);
		return NULL;
	}

	//======= set socket options =======
	int one = 1;

	//enable error detection on socket (for mftp_check_connection_error)
	result = setsockopt(sockfd,IPPROTO_IP,IP_RECVERR,&one,sizeof(int));
	if (result < 0){
		DEBUG_EXTRA perror("setsockopt");
		mftp_disconnect(connection);
		return NULL;
	}
	//enable instant reuse of this address if the socket is bound
	result = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(int));
	if (result < 0){
		DEBUG_EXTRA perror("setsockopt");
		mftp_disconnect(connection);
		return NULL;
	}

	return connection;
}
struct mftp_connection *mftp_connect(char *address, char *port){
	if (port == NULL){
		port = DEFAULT_MFTP_PORT;
	}
	struct mftp_connection *connection = mftp_create_connection(port);
	
	//====== get the address info for client ======
	struct addrinfo hints, *client_address_info;
	memset(&hints,0,sizeof(struct addrinfo));

	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_UNSPEC;

	int result = getaddrinfo(address,port,&hints,&client_address_info);
	if (result < 0){
		DEBUG_EXTRA fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		mftp_disconnect(connection);
		return NULL;
	}
	connection->connection_address_info = client_address_info;
	connection->connection_addr = malloc(client_address_info->ai_addrlen);
	memcpy(connection->connection_addr,client_address_info->ai_addr,client_address_info->ai_addrlen);
	connection->connection_addrlen = client_address_info->ai_addrlen;

	//====== handshake with server ======
	//hold out hand
	struct mftp_communication_chunk chunk;
	chunk.flags |= CHUNK_FLAG_CONNECTION_REQUEST;
	strcpy(chunk.data,"Connection request");
	result = mftp_send_communication_chunk(connection,&chunk,0);
	if (result < 0){
		DEBUG_EXTRA perror("mftp_send_communication_chunk");
		mftp_disconnect(connection);
		return NULL;
	}
	//wait for the client to shake
	struct pollfd poll_data;
	poll_data.fd = connection->socket;
	poll_data.events = POLLIN;
	
	result = poll(&poll_data,1,MAX_RESPONSE_TIMEOUT);
	if (result < 0){
		DEBUG_EXTRA perror("poll");
		mftp_disconnect(connection);
		return NULL;
	}
	if (result == 0){
		errno = ETIMEDOUT;
		DEBUG_EXTRA printf("connection timed out\n");
		mftp_disconnect(connection);
		return NULL;
	}

	//confirm its a handshake
	mftp_recv_communication_chunk(connection,&chunk,NULL,NULL,0);
	if (!((chunk.flags & CHUNK_FLAG_ACCEPT_CONNECTION) > 0)){
		//they declined
		errno = ECONNREFUSED;
		DEBUG_EXTRA printf("connection declined\n");
		mftp_disconnect(connection);
		return NULL;
	}
	
	return connection;
}
struct mftp_connection *mftp_listen(char *port){
	if (port == NULL){
		port = DEFAULT_MFTP_PORT;
	}
	struct mftp_connection *connection = mftp_create_connection(port);
	if (connection == NULL){
		return NULL;
	}
	int sockfd = connection->socket;

	//====== bind to selected port ======
	bind(
		sockfd,
		connection->host_address_info->ai_addr,
		connection->host_address_info->ai_addrlen
	);

	//====== find a client ======
	DEBUG_EXTRA printf("listening for connection...\n");
	socklen_t src_addrlen = sizeof(struct sockaddr_in6); //max size necessary
	struct sockaddr *src_addr = malloc(src_addrlen);
	for (int i = 0;;i++){
		struct mftp_communication_chunk chunk;
		int result = mftp_recv_communication_chunk(connection,&chunk,src_addr,&src_addrlen,0);
		if ((result < 0) || ((chunk.flags & CHUNK_FLAG_ACCEPT_CONNECTION) > 0)){
			if (i > MAX_CONNECTION_ATTEMPTS){
				DEBUG_EXTRA printf("max connection attempts reached");
				mftp_disconnect(connection);
				return NULL;
			}
			continue;
		}
		break;
	}

	//====== store it in the connection struct ======
	connection->connection_addr = src_addr;
	connection->connection_addrlen = src_addrlen;

	//====== send ack (MAX_CONNECTION_ATTEMTS times for good measure) ======
	struct mftp_communication_chunk chunk;
	chunk.flags |= CHUNK_FLAG_ACCEPT_CONNECTION;
	
	//packet is picked up as a repeat after first recipt
	mftp_timestamp_communication_chunk(&chunk);

	//for (int i = 0;i < MAX_CONNECTION_ATTEMPTS;i++){
		int result = mftp_send_communication_chunk(connection,&chunk,FLAG_DONT_TIMESTAMP);
		if (result < 0){
			DEBUG_EXTRA perror("mftp_send_communication_chunk");
			mftp_disconnect(connection);
			return NULL;
		}
	//}
	return connection;
}
int mftp_disconnect(struct mftp_connection *connection){
	DEBUG_EXTRA printf("disconnecting from connection.\n");
	
	//close socket
	close(connection->socket);

	//clean up allocated memory
	freeaddrinfo(connection->connection_address_info);
	freeaddrinfo(connection->host_address_info);
	free(connection->connection_addr);
	free(connection);
}
int mftp_connection_check_error(struct mftp_connection *connection){
	// im not gonnal lie i have no clue whats going on
	// i did write this but im not sure how it works
	//DEBUG_EXTRA printf("checking for errors...\n");
	int sockfd = connection->socket;
	size_t max_buffer_size = 512;
	char *anc_buffer = malloc(max_buffer_size);

	//man cmsg
	struct msghdr message;
	memset(&message,0,sizeof(struct msghdr));
	message.msg_controllen = max_buffer_size;
	message.msg_control = anc_buffer;
	
	int result = recvmsg(sockfd,&message,MSG_ERRQUEUE);
	//                   man recvmsg --- ^^^^^^^^^^^^
	if (result < 0){
		//acts as a non blocking socket when no errors are available
		if (errno == EWOULDBLOCK || errno == EAGAIN){
			//no errors
			//DEBUG_EXTRA printf("No errors found.\n");
			free(anc_buffer);
			return 0;
		}
		DEBUG_EXTRA perror("recvmsg");
		free(anc_buffer);
		return -1;
	}
	
	struct cmsghdr *anc_data;
	anc_data = CMSG_FIRSTHDR(&message);
	if (anc_data == NULL){
		DEBUG_EXTRA fprintf(stderr,"No room for header.\n");
		return -1;
	}
	char *data_buffer = malloc(sizeof(uint32_t));
	memcpy(data_buffer,CMSG_DATA(anc_data),sizeof(uint32_t));
	
	DEBUG_EXTRA printf("error found: %s\n",strerror(*((uint32_t *)data_buffer)));
	errno = *((uint32_t *)data_buffer);
	free(anc_buffer);
	free(data_buffer);
	return -1;
}
