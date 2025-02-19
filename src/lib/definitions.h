#ifndef _MFTP_CONSTANTS_H
#define _MFTP_CONSTANTS_H

//comment out to disable debug
#define MFTP_DEBUG_EXTRA

#include <netdb.h>
#include <sys/socket.h>

//====== return codes and flags ======
#define RETURN_SUCCESS 0
#define RETURN_ERROR_GENERAL -1

//function flags
#define FLAG_DONT_TIMESTAMP 1

//chunk flags
#define CHUNK_FLAG_CONNECTION_REQUEST 1
#define CHUNK_FLAG_ACCEPT_CONNECTION 2

//default values
#define DEFAULT_MFTP_PORT "1249"
#define MAX_CHUNK_DATA_SIZE 512
#define MAX_TIMESPEC_BACKLOG 5 //for detecting repeat packets
#define MAX_CONNECTION_ATTEMPTS 10
#define MAX_RESPONSE_TIMEOUT 1000 //miliseconds

//====== macros ======
#ifndef MFTP_DEBUG_EXTRA
#define DEBUG_EXTRA if(0)
#else
#define DEBUG_EXTRA if(1)
#endif
#define INIT_COMMUNICATION_CHUNK(chunk_p) do{(chunk_p)->flags=0;}while(0)

//====== structs ======
struct mftp_connection {
	struct addrinfo *host_address_info;
	struct addrinfo *connection_address_info;

	//addresses
	struct sockaddr *connection_addr;
	socklen_t connection_addrlen;
	
	//the socket
	int socket;

	//for packet duplication detection
	struct timespec previous_timestamps[MAX_TIMESPEC_BACKLOG];
	int previous_timestamps_oldest; //its like a circular queue but not
};
struct mftp_communication_chunk {
	//====== set by sending funciton =======
	struct timespec timestamp; //used for duplicate packet detection
	uint64_t flags;

	//====== set by user ======
	char data[MAX_CHUNK_DATA_SIZE];
};

//======== prototypes =========
struct mftp_connection *mftp_connect(char *address, char *port);
struct mftp_connection *mftp_listen(char *port);
struct mftp_connection *mftp_create_connection(char *port);
int mftp_disconnect(struct mftp_connection *connection);
int mftp_send_communication_chunk(struct mftp_connection *connection, struct mftp_communication_chunk *chunk,int flags);
int mftp_recv_communication_chunk(struct mftp_connection *connection,struct mftp_communication_chunk *chunk,struct sockaddr *src_addr,socklen_t *src_addrlen,int flags);
int mftp_connection_check_error(struct mftp_connection *connection);
int mftp_timestamp_communication_chunk(struct mftp_communication_chunk *chunk);

#endif
