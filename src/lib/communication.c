
#include "definitions.h"

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

//sending and receiving chunks
int mftp_send_communication_chunk(struct mftp_connection *connection, struct mftp_communication_chunk *chunk){
	//unpack the struct a little
	int sockfd = connection->socket;

	//set the chunk timestamp
	clock_gettime(CLOCK_REALTIME,&(chunk->timestamp));

	//send untill the receiver gets it
	for (;;){
		int result = sendto(
			sockfd,
			chunk,
			sizeof(struct mftp_communication_chunk),0,
			connection->connection_addr,
			connection->connection_addrlen
		);
		if (result < 0){
			DEBUG_EXTRA perror("sendto");
			return -1;
		}
		//DEBUG_EXTRA printf("%d bytes out of %lu sent\n",result,sizeof(struct mftp_communication_chunk));
		if (mftp_connection_check_error(connection) < 0){
			DEBUG_EXTRA fprintf(stderr,"socket error.\n");
			return -1;
		}
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
		//====== receive the data ======
		int result = recvfrom(
			sockfd,
			chunk,
			sizeof(struct mftp_communication_chunk),
			0,NULL,NULL
		);
		//====== check nothing went wrong ======
		if (result < 0){
			DEBUG_EXTRA perror("recvfrom");
			free(chunk);
			return NULL;
		}
		if (mftp_connection_check_error(connection) < 0){
			DEBUG_EXTRA fprintf(stderr,"socket error.\n");
			return NULL;
		}
		//====== check for duplicate packets ======
		int duplicate = 0;
		for (int i = 0; i < MAX_TIMESPEC_BACKLOG; i++){
			if (memcmp(connection->previous_timestamps+i,&(chunk->timestamp),sizeof(struct timespec)) == 0){
				duplicate = 1;
				DEBUG_EXTRA printf("duplicate packet discarded\n");
				break;
			}
		}
		if (!duplicate) break;
		//======= store the timestamp with the others we have received ======
		//             (up to a threshold of MAX_TIMESPEC_BACKLOG)
		memcpy(&(connection->previous_timestamps[connection->previous_timestamps_oldest]),&(chunk->timestamp),sizeof(struct timespec));
		connection->previous_timestamps_oldest++;
		connection->previous_timestamps_oldest %= MAX_TIMESPEC_BACKLOG; //wrap around the pointer
	}
	return chunk;
}
