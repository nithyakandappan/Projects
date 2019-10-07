/**For each token in the codebook
	Insert it into a hash table
For each next unmatched bits
	Search for it in the hash table
		If null, append another bit
		If not null, append token to output file **/

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


char * readjustDec(char * s);
char * readjustBigDec(char * s,int i);
char * nextTokenDec(char * contents, int i);
int convertBinToDec(char * string) ;
void makeHashArray(char * contents,int fsize);
char * nextForOutputString(char * contents,int i,int fsize);
char * generateOutputStringDec(char * contents, int fsize);
void decompress(char * fileName,char * codebookName);
off_t fileSizeDec(const char *filename);
int longestString(char * string, int fsize);
char * convertIfNecessaryDec(char * str);
int isDirDec(const char* target);
void recurseDecompress(char * folder, char * codebookName);
void initializeHashArray(char * codebookName);
int isFile(const char* name);

char** hashArray;

void recurseDecompress(char * folder, char * codebookName) {
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
			if (isDirDec(path)) {
				recurseDecompress(path,codebookName);    
			}
			else {
				// .hcz only 
				len = strlen(path);
				if ((len >= 5) && (strcmp(&(path[len-4]),".hcz")==0)) {
					decompress(path,codebookName);
				}
				else {
					printf("\n** not .hcz file, skipping = %s\n", path);
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

off_t fileSizeDec(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

void printArray(int size) {
	int i;
	for (i=0;i<=size-1;i++) {
		if (hashArray[i]!=NULL) {
			printf("index %d , string %s\n",i,hashArray[i]);
		}
	}
}

void initializeHashArray(char * codebookName) {
	int fdescCB=open(codebookName,O_RDONLY);
  	if(fdescCB < 0)
  	{
      		printf("\nError: open in_file (%s)\n", codebookName);
      		return;
  	}
	int fsizeCB=fileSizeDec(codebookName);
	char * contentsCB=(char *)malloc((fsizeCB+2)*sizeof(char));
	if (read(fdescCB, contentsCB, fsizeCB)==0) { //error
		return;
	}
	int maxSize=longestString(contentsCB,fsizeCB);
	hashArray=(char **)malloc(sizeof(char *)*pow(2,maxSize));
	makeHashArray(contentsCB,fsizeCB);
}

void decompress(char * fileName,char * codebookName) {
//	printArray(pow(2,maxSize));
	int fdescFN=open(fileName,O_RDONLY);
  	if(fdescFN < 0)
  	{
      		printf("\nError: open in_file (%s)\n", fileName);
      		return;
  	}

	int fsizeFN=fileSizeDec(fileName);
	if (strlen(fileName)<=4 ||(fileName[strlen(fileName)-1]!='z'&&fileName[strlen(fileName)-2]!='c'||fileName[strlen(fileName)-3]!='h'||fileName[strlen(fileName)-4]!='.')) {
		return;
	}
	char * contentsFN=(char *)malloc((fsizeFN+2)*sizeof(char));
	if (read(fdescFN, contentsFN, fsizeFN)==0) { //error
		return;
	}
	char * outputString=generateOutputStringDec(contentsFN,fsizeFN);
	char * newFileName=(char *)malloc(sizeof(char)*strlen(fileName)+5);
	strcpy(newFileName,fileName);
	newFileName[strlen(fileName)-4]='\0';
	int fileDescNew=open(newFileName, O_WRONLY|O_TRUNC|O_CREAT, 0744);
	ssize_t isSuccess=write(fileDescNew, outputString, strlen(outputString));
}

int longestString(char * contents, int fsize) {
	int i=2;
	int length=0;
	int numToken=0;
	while(i<fsize) {
		char * next=nextTokenDec(contents,i);
		int thisLength=strlen(next);
		if (numToken % 4==0) {
			if (length<thisLength) {
				length=thisLength;
			}
		}
		i=i+thisLength;
		numToken=numToken+1;
	}
	return length;
}

char * generateOutputStringDec(char * contents, int fsize) {
	char * outputString=(char *)malloc(sizeof(char)+1);
	outputString[0]='\0';
	int i=0;
	while(i<fsize) {
		char * nextBits=nextForOutputString(contents,i,fsize);
		int intVersionOfBits=convertBinToDec(nextBits);
		char * nextString=hashArray[intVersionOfBits];
		outputString=readjustBigDec(outputString,strlen(nextString)+1);
		strcat(outputString,nextString);
		i=i+strlen(nextBits);
	}
	return outputString;
}

char * nextForOutputString(char * contents,int i, int fsize) {
	char * bitsSoFar=(char*)malloc(11*sizeof(char));
	bitsSoFar[0]='\0';
	int lengthmod10=0;
	while(i<fsize) {
		if (strlen(bitsSoFar)>0) {
			int intVersionOfBits=convertBinToDec(bitsSoFar);
			if (hashArray[intVersionOfBits]!=NULL) {
				return bitsSoFar;
			}
			if (lengthmod10>9) {
				bitsSoFar=readjustDec(bitsSoFar); 
				lengthmod10=0;
			}
		}
		char* s=(char *)malloc(2*sizeof(char));
		s[0]=contents[i];
		s[1]='\0';
		strcat(bitsSoFar,s);
		free(s);
		i=i+1;
	}
	return bitsSoFar;
}
		

void makeHashArray(char * contents, int fsize) { 
	int i=2;
	int numToken=0;
	char * code;
	int intVersionOfCode;
	while(i<fsize) {
		char * next=nextTokenDec(contents,i);
		int length=strlen(next);
		if (numToken%4==0) {
			code=next;
			intVersionOfCode=convertBinToDec(next);	
		}
		else if (numToken%2==0) {
			next=convertIfNecessaryDec(next);
			hashArray[intVersionOfCode]=next;
		}
		i=i+length;
		numToken=numToken+1;
	}
}

char * convertIfNecessaryDec(char * str) {
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

int convertBinToDec(char * string) {
	int i=strlen(string)-1;
	int result=0;
	int multiplier=0;
	while(i>=0) {
		int x;
		if (string[i]=='0') {
			x=0;
		}
		else {
			x=1;
		}
		result=result+x*pow(2,multiplier);
		multiplier=multiplier+1;
		i=i-1;
	}
	return result;
}

char * nextTokenDec(char * contents, int i) {
	char * token=(char*)malloc(11*sizeof(char));
	if (isspace(contents[i])) {
		token[0]=contents[i];
		return token;
	}
	int lengthmod10=0;
	while(i<strlen(contents)) { //make sure to change this for buildCodebook too
			if (isspace(contents[i])) {
				return token;
			}	
			if (lengthmod10>9) {
				token=readjustDec(token); 
				lengthmod10=0;
			}
			char* s=(char *)malloc(2*sizeof(char));
			s[0]=contents[i];
			s[1]='\0';
			strcat(token,s);
			free(s);
			i=i+1;
	}
	
}

char * readjustBigDec(char * s,int i) {//i is for the line
	char* s1=(char *)malloc((i*sizeof(char))+(strlen(s)+1)*sizeof(char));
	strcpy(s1,s);
	free(s);
	return s1;
}

char * readjustDec(char * s) {
	char* s1=(char *)malloc((10*sizeof(char))+(strlen(s)+1)*sizeof(char));
	strcpy(s1,s);
	free(s);
	return s1;
}

int isDirDec(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}
