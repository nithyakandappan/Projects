# *********************************************
# *  214 Systems Programming   		      *
# *  Spring 2019                              *
# *********************************************
CC = gcc

# CCFLAGS = -Wall

all: WTF WTFserver

WTFserver_utils.o: WTFserver_utils.c WTFserver.h
	gcc $(CCFLAGS) -c WTFserver_utils.c -o WTFserver_utils.o -lcrypto -pthread

WTFserver: WTFserver.c WTFserver_utils.o WTFserver.h
	gcc $(CCFLAGS) WTFserver.c WTFserver_utils.o -o WTFserver -lcrypto -pthread

WTF_utils.o: WTF_utils.c WTF.h
	gcc $(CCFLAGS) -c WTF_utils.c -o WTF_utils.o -lcrypto

WTF: WTF.c WTF_utils.o WTF.h
	gcc $(CCFLAGS) WTF.c WTF_utils.o -o WTF -lcrypto

clean:
	rm -rf WTFserver_utils.o WTFserver WTF_utils.o WTF




