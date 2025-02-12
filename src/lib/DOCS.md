
# Debug info

for error output, add `#define MFTP_DEBUG_EXTRA` before your `#include "mftp.h"`.

# Connection and disconnection

## `struct mftp_connection *mftp_connect(char *address, char *port)`

Feed this function the address and port of the machine to connect to. Port can be left as NULL to use the default port of 1249. returns a struct that is freed when given to `mftp_disconnect()`
The caller of this function retains ownership of address and port. Returns NULL on failure

## `int mftp_disconnect(struct mftp_connection *connection)`

closes and frees an existing connection. `connection` is freed after this function is called so do not attemt to use it after.
