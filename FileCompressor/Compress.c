/**For each token in the codebook
	1. Insert it into a hash table
        2. For each token in the file
Search it in the hash table
Append bits that correspond to the token to the output file
**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fileCompressor.h"


struct LLnode {
	char* word;
	char * bitCode;
	struct LLnode* next;
};

off_t fileSizeC(const char *filename);
void compress(char * fileName,char * codebookName);
void recurseCompress(char * folder, char * codebookName);
void makeHashTable(char * contents, int size,int fsize);
char * generateOutputStringC(char * contents, int fsize,int hashSize);
struct LLnode * LLinsert(struct LLnode * head, char * word, char * bitCode);
char * LLsearch(struct LLnode * head, char * word);
int hashFunction(char* word, int size);
void hashInsert(char* word, char * bitCode, int size);
char * hashSearch(char* word, int size);
char * nextTokenC(char * contents, int i,int fsize);
char * readjustC(char * s);
char * readjustBigC(char * s,int i);
int countTabs(char * string);
char * convertIfNecessaryC(char * str);
int isDirC(const char* target);
void initializeHashTable(char * codebookName);
int isFile(const char* name);

struct LLnode ** hashTable;
char * outputString;
int hashSize;


/**int main(int argc, char** argv) {
//	compress1("testfolder/testfolder2/test5.txt","codebookTest.txt");
//	compress1("test2.txt","codebookTest2_2.txt");
	compress("sample.txt","HuffmanCodebook");
}**/

void recurseCompress(char * folder, char * codebookName) {
	DIR * dirp = opendir(folder);
	char * path=(char *)malloc(sizeof(char)*(strlen(folder)+2));
	strcpy(path,folder);
	int len;

	while (dirp) {
		int errno = 0;
		struct dirent * dp;
	   	if ((dp = readdir(dirp)) != NULL) {
			if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
				continue;
			}
			free(path);
			path=(char *)malloc(sizeof(char)*(strlen(folder)+strlen(dp->d_name)+2));
			char * copy=(char *)malloc(sizeof(char)*2);
			copy[0]='/';
			copy[1]='\0';
			strcpy(path,folder);
			strcat(path,copy); 
			strcat(path,dp->d_name); 
			if (isDirC(path)) {
				recurseCompress(path,codebookName);    
			}
			else {
				// skip .hcz file 
				len = strlen(path);
				if ((len >= 5) && (strcmp(&(path[len-4]),".hcz")==0)) {
					printf("\n** .hcz file detected, skipping = %s\n", path);
				}
				else {
					compress(path,codebookName);
				}
			}
	    	} else {
			if (errno == 0) {
				return;
		  	  	closedir(dirp);
				return;
	       	 	}
			return;
	     	   	closedir(dirp);
	  	}
	}
	return;
}

void printLL(struct LLnode * ptr) {
	while (ptr!=NULL) {
		printf(", %s ",ptr->word);
		ptr=ptr->next;
	}
}

void printHashTable(int size) {
	int i=0;
	while (i<size) {
		if (hashTable[i]!=NULL) {
			printf("\nindex %d: ",i);
			printLL(hashTable[i]);
			printf("\n");
		}
		i=i+1;
	}
}

void initializeHashTable(char * codebookName) {
	int fdescCB;
	int fsizeCB;
	ssize_t ret_in;    /* byte-count returned by read() */

	fdescCB=open(codebookName,O_RDONLY);
  	if(fdescCB < 0)
  	{
      		printf("\nError: open in_file (%s)\n", codebookName);
      		return;
  	}
	if ((fsizeCB=fileSizeC(codebookName)) == -1) {
    		printf("\nError: fileSizeC() from file: %s\n", codebookName);
		return;
	}
	char * contentsCB=(char *)malloc((fsizeCB+5)*sizeof(char));
	if (contentsCB==NULL) {
    		printf("\nError: malloc() for contentsCB \n");
		return;
	}
	if ((ret_in = read(fdescCB, contentsCB, fsizeCB)) == -1) { // error
    		printf("\nError: read() from file: %s\n", codebookName);
		return;
	}
	if (ret_in != fsizeCB) { // did not read all bytes from file
		printf("\nDid not read all bytes from file:\n");
    		printf("\n  file: %s; ret_in: %ld; fsizeCB: %d\n", codebookName, ret_in, fsizeCB);
		return;
	}
	if (ret_in == 0) { // nothing to read
    		printf("\nNothing to read from file: %s\n", codebookName);
		return;
	}

	int numOfTabs=countTabs(contentsCB);
	hashSize=(int)(numOfTabs*1.3);
	hashTable=(struct LLnode **)malloc((hashSize+1)*sizeof(struct LLnode *));
	makeHashTable(contentsCB,hashSize,fsizeCB);
}

void compress(char * fileName,char * codebookName) {
	int fdescFN;
	int fsizeFN;
  	ssize_t ret_in;    /* byte-count returned by read() */

	char * outputString;
	int len;

//	printHashTable(hashSize);
	fdescFN=open(fileName,O_RDONLY);
  	if(fdescFN < 0)
  	{
      		printf("\nError: open in_file (%s)\n", fileName);
      		return;
  	}
	if ((fsizeFN=fileSizeC(fileName)) == -1) {
    		printf("\nError: fileSize() from file: %s\n", fileName);
		return;
	}
	char * contentsFN=(char *)malloc((fsizeFN+5)*sizeof(char));
	if (contentsFN==NULL) {
    		printf("\nError: malloc() for contentsFN \n");
		return;
	}
	if ((ret_in = read(fdescFN, contentsFN, fsizeFN)) == -1) { // error
    		printf("\nError: read() from file: %s\n", fileName);
		return;
	}
	if (ret_in != fsizeFN) { // did not read all bytes from file
		printf("\nDid not read all bytes from file:\n");
    		printf("\n  file: %s; ret_in: %ld; fsizeFN: %d\n", fileName, ret_in, fsizeFN);
		return;
	}
	if (ret_in == 0) { // nothing to read
    		printf("\nNothing to read from file: %s\n", fileName);
		return;
	}
	outputString=generateOutputStringC(contentsFN,fsizeFN,hashSize);
	char * newFileName=(char *)malloc((sizeof(char)*strlen(fileName))+5);
	char * extension=(char *)malloc(sizeof(char)*6);
	extension[0]='.';
	extension[1]='h';
	extension[2]='c';
	extension[3]='z';
	extension[4]='\0';
	strcpy(newFileName,fileName);
	strcat(newFileName,extension);
	int fileDescNew=open(newFileName, O_WRONLY|O_TRUNC|O_CREAT, 0744);
  	if(fileDescNew < 0)
  	{
      		printf("\nError: open file for write (%s)\n", newFileName);
      		return;
  	}
	ssize_t isSuccess=write(fileDescNew, outputString, strlen(outputString));
	if (isSuccess == -1)
		printf("\nError: [write]...");
	return;
}

void makeHashTable(char * contents, int size,int fsize) { 
	int i=2;
	int numToken=0;
	char * code;
	int intVersionOfCode;
	while(i<fsize) {
		char * next=nextTokenC(contents,i,fsize);
		int length=strlen(next);
		if (numToken%4==0) {
			code=(char *)malloc(length*sizeof(char)+5);
			strcpy(code,next);
		}
		else if (numToken%2==0) {
			next=convertIfNecessaryC(next);
			hashInsert(next, code, size);
		}
		i=i+length;
		numToken=numToken+1;
	}
}

char * convertIfNecessaryC(char * str) {
	if (strcmp(str,WHITESPACECHARS_SPC_TOKEN)==0) {
		str[0]=' ';
	}
	else if (strcmp(str,WHITESPACECHARS_TAB_TOKEN)==0) {
		str[0]='\t';
	}
	else if (strcmp(str,WHITESPACECHARS_LF_TOKEN)==0) {
		str[0]='\n';
	}
	else if (strcmp(str,WHITESPACECHARS_VT_TOKEN)==0) {
		str[0]='\v';
	}
	else if (strcmp(str,WHITESPACECHARS_FF_TOKEN)==0) {
		str[0]='\f';
	}
	else if (strcmp(str,WHITESPACECHARS_CR_TOKEN)==0) {
		str[0]='\r';
	}
	else {
		return str;
	}
	str[1]='\0';
	return str;
}

char * generateOutputStringC(char * contents, int fsize,int hashSize) {
	/**char * **/outputString=(char *)malloc(sizeof(char)*2);
	outputString[0]='\0';
	int i=0;
	while(i<fsize) {
		char * next=nextTokenC(contents,i,fsize);
		char * bitCode=hashSearch(next, hashSize);
		if (bitCode == NULL) {
			printf("\n** Warning: could not find bitCode for the given word (token).\n"); 
		}
		else {
			int bigger=strlen(bitCode)+5;
			outputString=readjustBigC(outputString,bigger);
			int bitCodeLength=strlen(bitCode)+3;
			char * s=(char *)malloc(sizeof(char)*bitCodeLength);
			strcpy(s,bitCode);
			strcat(outputString,s);
		}
		i=i+strlen(next);
		free(next);
	}
	return outputString;
}

struct LLnode * LLinsert(struct LLnode * head, char * word, char * bitCode) {
	struct LLnode* new=(struct LLnode *)malloc(1*sizeof(struct LLnode));
	new->word=word;	
	new->bitCode=bitCode;
	new->next=head;
	head=new;
	return head;
}

char * LLsearch(struct LLnode * head, char * word) { //returns bitCode or -1 if the word is not there
	struct LLnode* ptr=head;
	while (ptr!=NULL) {
		if (strcmp(word,ptr->word)==0) {
			return ptr->bitCode;
		}
		ptr=ptr->next;
	}
	return NULL;
}

struct LLnode ** allocateHashTable(int size) {
	struct LLnode** hashTable=(struct LLnode **)malloc((size+1)*sizeof(struct LLnode *));
	return hashTable;
}

int hashFunction(char* word, int size) { 
	int length=strlen(word);
	int key=0;
	int i;
	for (i=0;i<length;i++) {
		key=key*13+(int)word[i];
		if (key>165190749) {
			key=key % size;
		}
	}
	return key % size;
}

void hashInsert(char* word, char * bitCode, int size) {
	int index=hashFunction(word,size);
	hashTable[index]=LLinsert(hashTable[index],word,bitCode);
}

char * hashSearch(char* word, int size) { //returns NULL if not there
	int index=hashFunction(word,size);
	char * s=LLsearch(hashTable[index],word);
	return s;
}

char * nextTokenC(char * contents, int i,int fsize) {
	char * token=(char*)malloc(11*sizeof(char));
	token[0]='\0';
	if (isspace(contents[i])) {
		token[0]=contents[i];
		token[1]='\0';
		return token;
	}
	int lengthmod10=0;
	while(i<fsize) { //make sure to change this for buildCodebook too
			if (isspace(contents[i])) {
				return token;
			}	
			if (lengthmod10>9) {
				token=readjustC(token); 
				lengthmod10=0;
			}
			char* s=(char *)malloc(3*sizeof(char));
			s[0]=contents[i];
			s[1]='\0';
			strcat(token,s);
			free(s);
			i=i+1;
	}
	return token;
	
}

char * readjustC(char * s) {
	char* s1=(char *)malloc((10*sizeof(char))+(strlen(s)+5)*sizeof(char));
	strcpy(s1,s);
	free(s);
	return s1;
}

char * readjustBigC(char * s,int i) {//i is for the line
	int size=i+strlen(s)+5;
	char * s1=(char *)malloc(size*sizeof(char));
	strcpy(s1,s);
	free(s);
	return s1;
	return s;
}

off_t fileSizeC(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

int countTabs(char * string) {
	int i;
	int tabCount=0;
	for (i=0;i<=strlen(string)-1;i++) {
		if (string[i]=='\t') {
			tabCount=tabCount+1;
		}
	}
	return tabCount;
}

int isDirC(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}
