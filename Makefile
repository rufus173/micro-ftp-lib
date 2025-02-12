CC=gcc
CFLAGS=-g
LIBS=-fsanitize=address

#auto generation
MFTP_MAIN_OBJ_LIST=$(shell find src/main/ | grep -E [.]c$$ | sed s/[.]c/.o/g)
MFTP_LIB_OBJ_LIST=$(shell find src/lib/ | grep -E [.]c$$ | sed s/[.]c/.o/g)

#MFTP_MAIN_OBJ_LIST=src/main/main.o
#MFTP_LIB_OBJ_LIST=

mftp : ${MFTP_MAIN_OBJ_LIST} ${MFTP_LIB_OBJ_LIST}
	$(CC) -o $@ $^ $(LIBS)
