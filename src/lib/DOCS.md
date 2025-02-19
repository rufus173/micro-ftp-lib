
# Debug info

For error output, add `#define MFTP_DEBUG_EXTRA` before your `#include "mftp.h"`.

# Connection and disconnection

Connection use this structure:
```C
struct mftp_connection {
	//===== dont worry about it =====
	struct addrinfo *host_address_info;
	struct addrinfo *connection_address_info;

	//addresses
	struct sockaddr *connection_addr;
	socklen_t connection_addrlen;

	//for packet duplication detection
	struct timespec previous_timestamps[MAX_TIMESPEC_BACKLOG];
	int previous_timestamps_oldest; //its like a circular queue but not
	
	//===== feel free to use this =====
	//the socket
	int socket;

};
```

## `struct mftp_connection *mftp_connect(char *address, char *port)`

Feed this function the address and port of the machine to connect to.
Port can be left as NULL to use the default port of 1249. Returns a struct that is freed when given to `mftp_disconnect()`
The caller of this function retains ownership of address and port. Returns NULL on failure

## `struct mftp_connection *mftp_listen(char *address, char *port)`

This function is similar to its TCP counterpart, but will automatically accept and return the relevant connection.

## `int mftp_disconnect(struct mftp_connection *connection)`

Closes and frees an existing connection. `connection` is freed after this function is called so do not attempt to use it after.

## `int mftp_connection_check_error(struct mftp_connection *connection)`

This function checks a connection to see if any ICMP messages about failed messages are available, hence whether any detectable errors have occurred. This does not detect if packets are dropped, or if the message gets to its location.
Returns 0 if no errors were found or -1 if they were. Automatically called in the send and receive functions provided by this library. Errno is set if an error is detected, so if you want, you can use `perror` when it fails

# Sending data

Sending data uses this structure:
```C
struct mftp_communication_chunk {
	//data used by the sending and receiving funciton
	...

	//data you as the user can set
	char data[MAX_CHUNK_DATA_SIZE];
};
```


## `int mftp_send_communication_chunk(struct mftp_connection *connection, struct mftp_communication_chunk *chunk,int flags)`

This function will send a communication chunk. The chunk provided should be owned by the caller, and should be mutable. This function does not guarantee that the data reaches the receiver, but does guarantee that the data will be valid if it arrives.
Return value of 0 on success, and -1 on failure.

## `int mftp_recv_communication_chunk(struct mftp_connection *connection,struct mftp_communication_chunk *,int flags)`

This function receives data from the connection. It will ensure there are no duplicate chunks and that any data that does arrive will be as it was sent.
Return value of 0 on success, and -1 on failure.

## `int mftp_timestamp_communication_chunk(struct mftp_communication_chunk *chunk)`

This is usefull with the flag `FLAG_DONT_TIMESTAMP`, as this allows you to send what the reciever would recognise as the "same" packet multiple times. The receiver weeds out duplicates based on duplicate timestamps, so by sending multiple times with the same timestamp, only one will be picked up.
