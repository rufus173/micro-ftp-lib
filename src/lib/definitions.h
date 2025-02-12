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

//====== macros ======
#ifndef MFTP_DEBUG_EXTRA
#define DEBUG_EXTRA if(0)
#else
#define DEBUG_EXTRA if(1)
#endif

//====== structs ======
struct mftp_connection {
	struct addrinfo *address_info;
	int socket;
};
struct communication_chunk {
};

//======== prototypes =========
struct mftp_connection *mftp_connect(char *address, char *port);
int mftp_disconnect(struct mftp_connection *connection);

#endif
