CC = gcc

CCFLAGS = -Wall

all: mymalloc.o memgrind

mymalloc.o: mymalloc.c mymalloc.h
	gcc $(CCFLAGS) -c mymalloc.c -o mymalloc.o

memgrind: memgrind.c mymalloc.o mymalloc.h
	gcc $(CCFLAGS) memgrind.c mymalloc.o -o memgrind

clean:
	rm -rf mymalloc.o memgrind 
