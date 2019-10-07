#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define malloc(x) mymalloc(x, __FILE__, __LINE__);
#define free(x) myfree(x, __FILE__, __LINE__);
void * mymalloc(size_t input, const char * file_name, int line_number);
void myfree(void *ptr, const char * file_name, int line_number);
