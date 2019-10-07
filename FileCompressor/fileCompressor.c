#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>

#include "fileCompressor.h"


void runProcesses(int op[],char* input,char* codebook);
int isDir(const char* target);
void buildCodebook(char * path,char * codebookName);
void initializeHashTable(char * codebookName);
void initializeHashArray(char * codebookName);
void compress(char * fileName,char * codebookName);
void recurseCompress(char * folder, char * codebookName);
void decompress(char * fileName,char * codebookName);
void recurseDecompress(char * folder, char * codebookName);
int isFile(const char* name);


int main(int argc, char* argv[]){
         if (argc<3){
                printf("Not enough arguments given.\n");
                return 0;
        }
	if (argc>5){
		printf("Too many arguments given.\n");
		return 0;
	}
        if ((strcmp(argv[1],"-R") != 0 )&& (strcmp(argv[1],"-b") != 0 )&&(strcmp(argv[1],"-c") != 0 )&&(strcmp(argv[1],"-d") != 0 )){
                printf("This is not a valid flag.\n");
                return 0;
        }
       
        int *op = (int*) calloc(4,sizeof(int));
        int i=1;
        while ((strcmp(argv[i],"-R") == 0 )|| (strcmp(argv[i],"-b") == 0 )||(strcmp(argv[i],"-c") == 0 )||(strcmp(argv[i],"-d") == 0 )){

                if(strcmp(argv[i],"-R")==0){
       //                 printf("op0 value: %d\n",op[0]);
                        if (op[0] == 1){
        //                      printf("op0 value: %d\n",op[0]);
                                printf("Incorrect arguments. Cannot use -R  multiples times.\n");
                                return 0;
                        }
                        op[0] = 1;

                } else if (strcmp(argv[i],"-b") ==0){
//                        printf("op1 value: %d\n",op[1]);
                        if (op[1] == 1){
                                printf("Incorrect arguments. Cannot use -b  multiples times.\n");
                                return 0;
                        }
                        op[1] = 1;
                } else if (strcmp(argv[i],"-c") ==0){
                        if (op[2] == 1){
                                printf("Incorrect arguments. Cannot use -c  multiples times.\n");
                                return 0;
                        }
                        op[2] = 1;
                } else if(strcmp(argv[i],"-d") ==0){
                        if (op[3] == 1){
                                printf("Incorrect arguments. Cannot use -d  multiples times.\n");
                                return 0;
                        }
                        op[3] = 1;
                }
                i++;
        }
        if ((op[1]==1 && op[2]==1) || (op[1]==1 && op[3]==1) || op[2]==1 && op[3]==1){
                printf("Error: These flags cannot be used together. Only -R can be used with another flag.\n");
                return 0;
        }
        char* input;
        char* codebook = NULL;
	input = (char*)malloc(strlen(argv[i]));
	input = argv[i];
	i++;
	if (argc>i){
		codebook = (char*)malloc(strlen(argv[i]));
		codebook = argv[i];
	}
        runProcesses(op,input,codebook);
}

void runProcesses(int op[], char* input, char* codebook){

        struct dirent *de;
        DIR *dr;
	int len;

     	if ((op[0]==1)&&(!isDir(input))){
                printf("Warning: You provided the recursive argument but did not provide a directory to recurse on.\n");
        }
	if ((op[0]==0)&&(isDir(input))) {
		printf("Warning: You did not provide the recursive argument but provided a directory to recurse on.\n");
	}
        if (op[1] == 1){
		// return 1 :  file
		// return 0 :  directory
		// return -1 : invalid name
		if (isFile(input) == -1) {
    			printf ("\n** Error: invalid input = |%s|\n", input);
    			return;
		}
		if (codebook != NULL){
			printf("Error: Given a codebook when one is not needed for buildCodebook.\n");
			return;
		}
		buildCodebook(input,"HuffmanCodebook");
		return;
        }
	if (op[2] == 1){
		if (isFile(input) == -1) {
    			printf ("\n** Error: invalid input = |%s|\n", input);
    			return;
		}
		if (isFile(codebook) != 1) {
    			printf ("\n** Error: codebook |%s| is not a valid file.\n", codebook);
    			return;
		}
		initializeHashTable(codebook);
		if (isDir(input)) {
			recurseCompress(input,codebook);
		}
		else {
			// skip .hcz
			len = strlen(input);
			if ((len >= 5) && (strcmp(&(input[len-4]),".hcz")==0)) {
				printf("\n** .hcz file detected, skipping = %s\n", input);
				return;
			}
			compress(input,codebook);
		}
	}
	if (op[3] == 1){
		if (isFile(input) == -1) {
    			printf ("\n** Error: invalid input = |%s|\n", input);
    			return;
		}
		if (isFile(codebook) != 1) {
    			printf ("\n** Error: codebook |%s| is not a valid file.\n", codebook);
    			return;
		}
		initializeHashArray(codebook);
		if (isDir(input)) {
			recurseDecompress(input,codebook);
		}
		else {
			// .hcz only 
			len = strlen(input);
			if ((len >= 5) && (strcmp(&(input[len-4]),".hcz")==0)) {
				decompress(input,codebook);
			}
			else {
				printf("\n** not .hcz file, skipping = %s\n", input);
				return;
			}
		}
	}

}

int isDir(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}
