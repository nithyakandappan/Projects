# *********************************************
# *  214 Systems Programming   		      *
# *  Spring 2019                              *
# *********************************************
CC = gcc

# CCFLAGS = -Wall

all: fileCompressor

Utils.o: Utils.c fileCompressor.h
	gcc $(CCFLAGS) -c Utils.c -o Utils.o -lm

BuildCodebook.o: BuildCodebook.c fileCompressor.h
	gcc $(CCFLAGS) -c BuildCodebook.c -o BuildCodebook.o -lm

Compress.o: Compress.c fileCompressor.h
	gcc $(CCFLAGS) -c Compress.c -o Compress.o -lm

Decompress.o: Decompress.c fileCompressor.h
	gcc $(CCFLAGS) -c Decompress.c -o Decompress.o -lm

fileCompressor: fileCompressor.c BuildCodebook.o Compress.o Decompress.o Utils.o fileCompressor.h
	gcc $(CCFLAGS) fileCompressor.c BuildCodebook.o Compress.o Decompress.o Utils.o -o fileCompressor -lm

clean:
	rm -rf Utils.o BuildCodebook.o Compress.o Decompress.o fileCompressor
