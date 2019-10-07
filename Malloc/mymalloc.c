#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "mymalloc.h"


static char myblock[4096];

void merge(short int i,short int previ);


void * mymalloc(size_t input, const char * file_name, int line_number) {
   int i, j;
   short int size;

   // check input
   if (input > 4096) {
     printf("[%s:%d] error: not enough available space in memory\n", file_name, line_number);
     return NULL;
   }
   else if (input <= 0) {
     printf("[%s:%d] error: input size is not a positive integer\n", file_name, line_number);
     return NULL;
   }
   size = (short int) input;

   // if it is the first entry
   if (((short int *) &myblock[1])[0] == 0)
   {
     // initialize
     ((short int *) &myblock[1])[0] = 4096 - 3;
     ((char *) &myblock[0])[0] = '\0';
     // check size requested
     if (size > (4096 - 3))
     {
	printf("[%s:%d] error: not enough available space in memory\n", file_name, line_number);

        return NULL;
     }
     // grant the space (size)
     ((char *) &myblock[0])[0] = 'n';
     if (size >= (4096 - 6))	// can allocate space but cannot or is not worth adding a new metadata block
     {
       ((short int *) &myblock[1])[0] = 4096 - 3;
     }
     else   // size < (4096 - 6)
     {
       ((short int *) &myblock[1])[0] = (short int) size;
       // initialize the next metadata block
       j = 0 + ((short int *) &myblock[1])[0] + 3;    // [j] is the index for the beginning of the next metadata block
       ((char *) &myblock[j])[0] = '\0';
       ((short int *) &myblock[j+1])[0] = 4096 - ((short int *) &myblock[1])[0] - 6;	// ***
     }
     return &myblock[3];
   }
   else
   {
     // this is not the first call to malloc, search for the first fit to allocate space for "size"
     i=0;						// index for the beginning of the current metadata block
     while (i < 4096)
     {

       if (((char *) &myblock[i])[0] == '\0') 		// if the current block is free
       {

         if (size < (((short int *) &myblock[i+1])[0] - 3))    // can allocate space and make a new metadata after
         {

	   // below must be done in order
           j = i + size + 3;        			  // [j] is the index for the beginning of the next metadata in the array
           ((char *) &myblock[j])[0] = '\0';		  // set the beginning of the next metadata block to be "free"
	   ((short int *) &myblock[j+1])[0] = ((short int *) &myblock[i+1])[0] - size - 3;  // the algorithm makes sure that this is at least 1, worth making a new metadata block
	   ((short int *) &myblock[i+1])[0] = (short int) size;	// adjust the current block size to be the size requested
           ((char *) &myblock[i])[0] = 'n';		  	  	// set the byte at the beginning of the current metadata to be "not free"
           return &myblock[i+3];
         }
         else if ((size >= (((short int *) &myblock[i+1])[0] - 3)) &&   	// you can allocate space and it is not worth making a new metadata after
	          (size <= ((short int *) &myblock[i+1])[0]))
         {

           ((char *) &myblock[i])[0] = 'n';		  	  	// set the beginning of the current metadata block to be "not free"
           return &myblock[i+3];
         }
       }
       // try to advance i to reach the next block.

       i = i + ((short int *) &myblock[i + 1])[0] + 3;      // if the program is already in the last block, after advancing, i=4096, so stop the loop
     }

     printf("[%s:%d] error: not enough available space in memory\n", file_name, line_number);
     return NULL;
   }
}


void myfree(void * p, const char * file_name, int line_number) { //frees block of memory to which the pointer is pointing

	//error message if the array is not yet initialized
	if (((short int *) &myblock[1])[0] == 0) {
		printf("[%s:%d] error: The input pointer was not allocated by malloc(). The array is not initialized yet.\n", file_name, line_number);
		return;
	}
	//error message if the pointer was not allocated by malloc
	if (p < (void *) myblock || p > (void *) (myblock+4096)) {
		printf("[%s:%d] error: The input pointer was not allocated by malloc().\n", file_name, line_number);
		return;
	}
	short int i=0;
	short int previ=-1;
	while (i<4096) {
		if (p == (void *)&myblock[i+3]) { //pointer is at a proper position
			if (((char *) &myblock[i])[0]=='n') { //successful free. 'n': not free, '\0: free
				((char *) &myblock[i])[0]='\0';
			merge(i,previ);
			return;
			}
			else {
				printf("[%s:%d] error: This is a redundant free, the space allocated by your pointer is already free\n", file_name, line_number);
				return;
			}
		}
		else if (p < (void *)&myblock[i+3]) { //pointer is in the middle of metadata or data, not at the right position
			printf("[%s:%d] error: Your argument was not a pointer that was allocated by malloc()\n", file_name, line_number);
			return;
		}
		previ = i;
		i = i + ((short int *) &myblock[i+1])[0] + 3;
	}
	printf("[%s:%d] error: Your argument was not a pointer that was allocated by malloc\n", file_name, line_number);
	return;
}

void merge(short int i,short int previ) { //helper function to merge free blocks
	short int j;

	// [i] is the index in array for the beginning of the current metadata
	// [j] is the index in array for the beginning of the next metadata
	j = i + ((short int *) &myblock[i+1])[0] + 3;

	// merge the current and next block
	if (j < 4096) {
		if ((char)myblock[j]=='\0') {
			((short int *) &myblock[i+1])[0] = ((short int *) &myblock[i+1])[0] + ((short int *) &myblock[j+1])[0] + 3;
		}
	}
	// merge the current and previous  block
	if (previ>-1) {
		if ((char)myblock[previ]=='\0') {
			((short int *) &myblock[previ+1])[0] = ((short int *) &myblock[previ+1])[0] + ((short int *) &myblock[i+1])[0] + 3;

		}
	}
}
