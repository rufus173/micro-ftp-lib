CC=gcc
CFLAGS=-g
LIBS=
MFTP_MAIN_OBJ=src/main/main.o
MFTP_LIB_OBJ=

mftp : ${MFTP_MAIN_OBJ} ${MFTP_LIB_OBJ}
	$(CC) -o $@ $^ $(LIBS)
