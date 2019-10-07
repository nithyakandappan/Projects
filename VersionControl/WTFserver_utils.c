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
#include <errno.h>
#include <openssl/sha.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>                      
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <pwd.h>

#include "WTFserver.h"

enum { PORTSIZE = 6 };

struct LLnode {
	char * path;
	char * vNum;
	char * hash;
//	char * projName;
	struct LLnode* next;
};

off_t fileSize(const char *filename);
char * getFileSHA1(char * fileName);
int mkpath(char* file_path, mode_t mode);
int get_sockfd(char * server_hostname, unsigned short server_port);
int receive1file(int connect_socket, char * filePath_toSave);
int send1file(int sockfd, char * file_path);
int isDir(const char* target);
char * nextToken(char * contents, int i,int fsize);
char * readjust(char * s);
char *str_replace(char *orig, char *rep, char *with);
char * readjustBig(char * s,int i);
void reverse(char s[]);
void itoa(int n, char s[]);
int isFile(const char* name);
const char *getUserName();
char* createFilePath(char* projectname,char* fileName);
char* readFileDatafromFileName(char *filePath);
int create_file(char * logPath);
void removingDirectory(char* projectPath);

struct LLnode * LLsearch(struct LLnode * head, char * word);
struct LLnode * LLinsert(struct LLnode * head, char * path, char * vNum, char * hash);
struct LLnode * LLdelete(struct LLnode * head, char * path);

int create_file(char * logPath) 
{
  int fd2 = open(logPath, O_RDWR|O_CREAT|O_APPEND, 0777);

  if (fd2 != -1) {
	close(fd2);
	return 0;
  }
  return 1;
}

const char *getUserName()
{
  uid_t uid = geteuid();
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}

// return 1 :  file
// return 0 :  directory
// return -1 : invalid name
int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     return 0;
    }

    if(errno == ENOTDIR)
    {
     return 1;
    }

    return -1;
}


void removingDirectory(char* projectPath)
{
    if (isFile(projectPath) == 0){
        char path[100];
        strcpy(path,projectPath);
        strcat(path,"/");
        DIR *dir;
        dir = opendir(projectPath);

        //move through directory entries
        struct dirent *file;
        while ((file = readdir(dir)) != NULL){
            if ((strcmp(file->d_name, ".")) || (strcmp(file->d_name,".."))){
                continue;
            } else {
                strcat(path,file->d_name);
                removingDirectory(path);
            }
        }
    }
    remove(projectPath);

    return;
}


/* reverse:  reverse string s in place */
void reverse(char s[])
{
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}  

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

struct LLnode * LLdelete(struct LLnode * head, char * path) {
	struct LLnode* traverse=head;
	struct LLnode* prev=NULL;
	while (traverse!=NULL) {
		if (strcmp((*traverse).path, path) == 0) {
			if (prev==NULL) {
				head=(*traverse).next;
				free((void *)traverse);
				return head;
			}
			else {
				(*prev).next=(*traverse).next;
				free((void *)traverse);
				return head;
			}
		}
		prev=traverse;
		traverse=(*traverse).next;
	}
	return head;
}

struct LLnode * LLinsert(struct LLnode * head, char * path, char * vNum, char * hash) 
{
	struct LLnode* new=(struct LLnode *)malloc(1*sizeof(struct LLnode));
	new->path=path;	
	new->vNum=vNum;
	new->hash=hash;
	new->next=head;
	head=new;

	return head;
}

struct LLnode * LLsearch(struct LLnode * head, char * word) { 
	struct LLnode* ptr=head;
	while (ptr!=NULL) {
		if (strcmp(word,ptr->path)==0) {
			return ptr;
		}
		ptr=ptr->next;
	}
	return NULL;
}

off_t fileSize(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

char * readjustBig(char * s,int i) {//i is for the line
	char* s1=(char *)malloc((i*sizeof(char))+(strlen(s)+1)*sizeof(char));
	strcpy(s1,s);
	free(s);
	return s1;
}

char * readjust(char * s) {
	char* s1=(char *)malloc((10*sizeof(char))+(strlen(s)+1)*sizeof(char));
	strcpy(s1,s);
	free(s);
	return s1;
}

char * nextToken(char * contents, int i,int fsize) {
	char * token=(char*)malloc(11*sizeof(char));
	
	token[0]='\0';
	if (isspace(contents[i])) {
		token[0]=contents[i];
		token[1]='\0';
		return token;
	}
	int lengthmod10=0;
	while(i<fsize) {
			if (isspace(contents[i])) {
				return token;
			}	
			if (lengthmod10>9) {
				token=readjust(token); 
				lengthmod10=0;
			}
			char* s=(char *)malloc(3*sizeof(char));
			s[0]=contents[i];
			s[1]='\0';
			strcat(token,s);
			free(s);
			i=i+1;
			lengthmod10++;
	}
	return token;
	
}

int isDir(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}

// before create a file, call mkpath first to create the path if needed
// use relative path such as 
//   "project1/v1/subdir/program.c", or 
//   "project1/v1/.manifest"
int mkpath(char* file_path, mode_t mode) {
  char* p;
  for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
    *p='\0';
    if (mkdir(file_path, mode)==-1) {
      if (errno!=EEXIST) { *p='/'; return -1; }
    }
    *p='/';
  }
  return 0;
}

char * getFileSHA1(char * fileName)
{
	int fdescFN;
	int fsizeFN;
  	ssize_t ret_in;    /* byte-count returned by read() */

	fdescFN=open(fileName,O_RDONLY);
  	if(fdescFN < 0)
  	{
      		printf("\nError: open in_file (%s)\n", fileName);
      		return NULL;
  	}
	if ((fsizeFN=fileSize(fileName)) == -1) {
    		printf("\nError: fileSize() from file: %s\n", fileName);
      		return NULL;
	}
	char * contentsFN=(char *)malloc(fsizeFN*sizeof(char));
	if (contentsFN==NULL) {
    		printf("\nError: malloc() for contentsFN \n");
      		return NULL;
	}
	if ((ret_in = read(fdescFN, contentsFN, fsizeFN)) == -1) { // error
    		printf("\nError: read() from file: %s\n", fileName);
      		return NULL;
	}
	if (ret_in != fsizeFN) { // did not read all bytes from file
		printf("\nDid not read all bytes from file:\n");
    		printf("\n  file: %s; ret_in: %ld; fsizeFN: %d\n", fileName, ret_in, fsizeFN);
      		return NULL;
	}
	if (ret_in == 0) { // nothing to read
    		printf("\nNothing to read from file: %s\n", fileName);
      		return NULL;
	}

	unsigned char hash[SHA_DIGEST_LENGTH];
	int i;
	SHA1(contentsFN, fsizeFN, hash); // hash now contains the 20-byte SHA-1 hash

	char * SHA1_key = (char *)malloc(sizeof(char)*100);
 	SHA1_key[0]='\0';
	char tmpStr[10]="\0";
       	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		tmpStr[0]='\0';
         	sprintf(tmpStr,"%.2x", hash[i]);
		strcat(SHA1_key,tmpStr);
       	}
	return SHA1_key;
}

int get_sockfd(char * server_hostname, unsigned short server_port)
{
    struct addrinfo hints, *res;
    struct hostent *hostent;
    int sockfd;
    char portNum[PORTSIZE];

    /* Prepare hint (socket address input). */
    hostent = gethostbyname(server_hostname);
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", server_hostname);
	return -1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // ipv4
    hints.ai_socktype = SOCK_STREAM;    // socket type, tcp
    hints.ai_flags = AI_PASSIVE;        // fill in my IP

    sprintf(portNum, "%d", server_port);
    getaddrinfo(NULL, portNum, &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
	return -1;
    }

    /* make connection. */
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        return -1;
    }

    return sockfd;
}

// =====================================================================
// ** implement function "receive1file"
//    1) hand shake with the other party via socket (sockfd)
//       sender send message to receiver first, with
//            (1) file size and (2) file name first
//       then receiver parse the message, to get  
//            (1) file size and (2) file name first
//    2) then both parties use write() and read() 
//	 to transmit the byte stream of the file 
//       via socket (sockfd)
// ** notice the parameter "filePath_toSave" in 
//      receive1file(int sockfd, char * filePath_toSave) 
//    * if "filePath_toSave" is NULL, or an empty string "", 
//      use filename from the sender to create file to save.
//    * if "filePath_toSave" is not NULL, nor an empty string "",  
//      use the parameter "filePath_toSave" to create file to save, 
//      ignore the filename sent from the sender
// ** char * filePath_toSave is a file with path
//    example: project1/subdir1/subdir2/data1.c
// ** this function can be used by both client and server
//    to receive one file from the other party
//    the funtion handles communication with the other party
// ** assuming the other party will 
//    call send1file(..)
//    to communicate with this receive1file(..) 
// =====================================================================
int receive1file(int connect_socket, char * filePath_toSave)
{
    int filefd;
    ssize_t read_return;
    char buffer[BUFSIZ];
    char *file_path;
    char file_path_adjusted[BUFSIZ];
    long long file_length;
    char receiveFileName[BUFSIZ];
    char rep[BUFSIZ];
    char with[BUFSIZ];
    char * result;
    char * result2;

    if (recv(connect_socket, receiveFileName, sizeof(receiveFileName), 0)) {

        // NOTE: now we have the client send us the file length so we
        // know when to stop the read loop below
        file_length = strtoll(receiveFileName,&file_path,10);

        if (*file_path != ',') {
            fprintf(stderr,"syntax error in request -- '%s'\n",
                receiveFileName);
            exit(EXIT_FAILURE);
        }
        file_path += 1;

        fprintf(stderr, "is the file name received? ?   =>  %s [%lld bytes]\n",
            file_path,file_length);

	// apply filePath_to_saveFile if specified
    	if ((filePath_toSave != NULL) && (strcmp(filePath_toSave,"") != 0)) {
	    strcpy(file_path_adjusted, filePath_toSave);
            fprintf(stderr,"\nfile_path_adjusted =|%s|\n", file_path_adjusted);
    	}
	else {
    	    printf("\nfile_path = |%s|\n", file_path);

	    strcpy(rep, "Repository/");
	    strcpy(with, "");
	    result = str_replace(file_path, rep, with);
	    printf("\nresult = |%s|\n", result);

	    strcpy(rep, "master/");
	    strcpy(with, "");
	    result2 = str_replace(result, rep, with);
	    printf("\nresult2 = |%s|\n", result2);

	    strcpy(file_path_adjusted, result2);
            fprintf(stderr,"\nfile_path_adjusted =|%s|\n", file_path_adjusted);

	    free(result);
	    free(result2);
	}

	// call mkpath(char* file_path, mode_t mode) 
        // to create path if it does not exist
	mode_t mode=0755;
	if (mkpath(file_path_adjusted, mode) != 0) {
            fprintf(stderr,"\nError: mkpath(file_path_adjusted, mode), file_path_adjusted=|%s|\n",
			file_path_adjusted);
	    return 1;
	}

        filefd = open(file_path_adjusted,
            O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (filefd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

     
        for (;  file_length > 0;  file_length -= read_return) {
            read_return = BUFSIZ;
            if (read_return > file_length)
                read_return = file_length;

            read_return = read(connect_socket, buffer, read_return);
            if (read_return == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            if (read_return == 0)
                break;

            if (write(filefd, buffer, read_return) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }

        fprintf(stderr,"file complete\n");

        close(filefd);
    }

    return 0;
}

// ===========================================================
// ** implement function "send1file"
//    1) hand shake with the other party via socket (sockfd)
//       sender send message to receiver first, with
//            (1) file size and (2) file name first
//       then receiver parse the message, to get  
//            (1) file size and (2) file name first
//    2) then both parties use write() and read() 
//	 to transmit the byte stream of the file 
//       via socket (sockfd)
// ** this function can be used by both client and server
//    to send one file to the other party
//    the funtion handles communication with the other party
// ** assuming the other party will 
//    call receive1file(..)
//    to communicate with this send1file(..) 
// ** char * file_path is the file to send with path
//    example: project1/subdir1/subdir2/data1.c
// ===========================================================
int send1file(int sockfd, char * file_path) 
{
	int filefd;
    	char remote_file[BUFSIZ];
    	struct stat st;
	char buffer[BUFSIZ];
    	ssize_t read_return;
	int return_status=0;

    	// NOTE: check the file _before_ sending request to server
    	// to know the file length to send to the server 
	filefd = open(file_path, O_RDONLY);
	if (filefd == -1) {
		perror("open send file");
		return 1;	
	}

	// get the file's byte length
	if (fstat(filefd,&st) < 0) {
		perror("stat send file");
		close(filefd);
		return 1;
	}

	// send file name to server
	memset(remote_file, 0, sizeof(remote_file));

	sprintf(remote_file, "%lld,%s",
		(long long) st.st_size,file_path);

	send(sockfd, remote_file, sizeof(remote_file), 0);

	while (1) {
		read_return = read(filefd, buffer, BUFSIZ);
		if (read_return == 0)
			break;

		if (read_return == -1) {
                        perror("read");
			return_status = 1;
                        break;
		}

		if (write(sockfd, buffer, read_return) == -1) {
			perror("write");
			return_status = 1;
                        break;
                    }
	}
        close(filefd);
	return return_status;
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

char* createFilePath(char* projectname,char* fileName){
        char *filePath = (char*)malloc(20+strlen(projectname)+strlen(fileName));
        strcpy(filePath,"Repository/");
        strcat(filePath,projectname);
        strcat(filePath,"/master");
        strcat(filePath,fileName);
        return filePath;
}


char* readFileDatafromFileName(char *filePath){
        struct stat sb;
        int size;
        char *fileData;

        lstat(filePath,&sb);
        size = sb.st_size;

        fileData = (char *) malloc(size + 10);

        //open and read file
        int fd = open(filePath,O_RDWR);
        read(fd,fileData,size);
	fileData[size] = '\0';
	close(fd);

        return fileData;
}



