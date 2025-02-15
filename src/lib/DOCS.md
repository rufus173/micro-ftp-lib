
# Debug info

For error output, add `#define MFTP_DEBUG_EXTRA` before your `#include "mftp.h"`.

# Connection and disconnection

## `struct mftp_connection *mftp_connect(char *address, char *port)`

Feed this function the address and port of the machine to connect to. Port can be left as NULL to use the default port of 1249. Returns a struct that is freed when given to `mftp_disconnect()`
The caller of this function retains ownership of address and port. Returns NULL on failure

## `int mftp_disconnect(struct mftp_connection *connection)`

Closes and frees an existing connection. `connection` is freed after this function is called so do not attempt to use it after.

## `int mftp_connection_check_error(struct mftp_connection *connection)`

This function checks a connection to see if any ICMP messages about failed messages are available, hence whether any detectable errors have occurred. This does not detect if packets are dropped, or if the message gets to its location.
Returns 0 if no errors were found or -1 if they were. Automatically called in the send and receive functions provided by this library.
