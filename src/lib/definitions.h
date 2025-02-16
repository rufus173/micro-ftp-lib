#ifndef _MFTP_CONSTANTS_H
#define _MFTP_CONSTANTS_H

//comment out to disable debug
#define MFTP_DEBUG_EXTRA

#include <netdb.h>
#include <sys/socket.h>

//====== return codes ======
#define RETURN_SUCCESS 0
#define RETURN_ERROR_GENERAL -1

//default values
#define DEFAULT_MFTP_PORT "1249"
#define MAX_PACKET_PAYLOAD_SIZE 512
#define MAX_TIMESPEC_BACKLOG 5 //for detecting repeat packets

//====== macros ======
#ifndef MFTP_DEBUG_EXTRA
#define DEBUG_EXTRA if(0)
#else
#define DEBUG_EXTRA if(1)
#endif

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
	//====== set by user ======
	char data[MAX_PACKET_PAYLOAD_SIZE];
};

//======== prototypes =========
struct mftp_connection *mftp_connect(char *address, char *port);
struct mftp_connection *mftp_listen(char *port);
struct mftp_connection *mftp_create_connection(char *port);
int mftp_disconnect(struct mftp_connection *connection);
int mftp_send_communication_chunk(struct mftp_connection *connection, struct mftp_communication_chunk *chunk);
struct mftp_communication_chunk *mftp_recv_communication_chunk(struct mftp_connection *connection);
int mftp_connection_check_error(struct mftp_connection *connection);

#endif
