
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

int mftp_timestamp_communication_chunk(struct mftp_communication_chunk *chunk){
	return clock_gettime(CLOCK_REALTIME,&(chunk->timestamp));
}

//sending and receiving chunks
int mftp_send_communication_chunk(struct mftp_connection *connection, struct mftp_communication_chunk *chunk,int flags){
	//unpack the struct a little
	int sockfd = connection->socket;

	//set the chunk timestamp
	if (!( (flags & FLAG_DONT_TIMESTAMP) > 0 )){
		DEBUG_EXTRA printf("chunk requested not to be timestamped\n");
		mftp_timestamp_communication_chunk(chunk);
	}

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
int mftp_recv_communication_chunk(struct mftp_connection *connection,struct mftp_communication_chunk *chunk,struct sockaddr *src_addr,socklen_t *src_addrlen,int flags){
	int sockfd = connection->socket;

	for (;;){
		//====== receive the data ======
		int result = recvfrom(
			sockfd,
			chunk,
			sizeof(struct mftp_communication_chunk),
			0,src_addr,src_addrlen
		);
		//====== check nothing went wrong ======
		if (result < 0){
			DEBUG_EXTRA perror("recvfrom");
			free(chunk);
			return -1;
		}
		if (mftp_connection_check_error(connection) < 0){
			DEBUG_EXTRA fprintf(stderr,"socket error.\n");
			return -1;
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
	return 0;
}
