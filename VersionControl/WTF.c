// ================================================
// client.c
// = Interface:
// ./executable send1file2server <filename>
// 	./client send1file2server test01.txt
// ./executable configure <ip> <port>
// 	./client configure 127.0.0.1 12345
// ./executable create <projectname>
// ================================================
#define _XOPEN_SOURCE 700

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

#include "WTF.h"

#define MAXFILES        1000

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
char * readjustBig(char * s,int i);
char *str_replace(char *orig, char *rep, char *with);
void itoa(int n, char s[]);
int isFile(const char* name);
const char *getUserName();
char * readFileDatafromFileName(char * filePath);		
char * createFilePath(char* projectname, char* fileName);	
void removingDirectory(char* projectPath);
void  LLfromManifest(struct LLnode **head,char *file);

struct LLnode * LLsearch(struct LLnode * head, char * word);
struct LLnode * LLinsert(struct LLnode * head, char * path, char * vNum, char * hash);
struct LLnode * LLdelete(struct LLnode * head, char * path);

int process_configure(char * ip, char * port); 			
int process_create(int sockfd, char * projectname); 		
int process_checkout(int sockfd, char * projectname);
int process_update(int sockfd, char * projectname);
int process_upgrade(int sockfd, char * projectname);
int process_commit(int sockfd, char * projectname);
int process_push(int sockfd, char * projectname);
int process_destroy(int sockfd, char * projectname);
int process_currentversion(int sockfd, char * projectname);
int process_history(int sockfd, char * projectname);
int process_add(int sockfd, char * projectname, char * filename);
int process_remove(int sockfd, char * projectname, char * filename);
int process_rollback(int sockfd, char * projectname, char * versionNumber);
int print_current_manifest(char *filePath);			


// NOTE2: the "volatile" attribute here is critical to proper operation
volatile int signo_taken;


void sig_handler(int signo)
{
    // just alert the base layer and let it handle things when it's in a
    // "safe" state to do so ...
    signo_taken = signo;
}

int main(int argc, char **argv)
{
    int sockfd;

    if (argc < 3) {
	// client only operation:
        fprintf(stderr, "sample usage: ./client configure <ip> <port>\n");
	// client and server operation:
        fprintf(stderr, "sample usage: ./client create <projectname>\n");
        fprintf(stderr, "sample usage: ./client destroy <projectname>\n");
        fprintf(stderr, "sample usage: ./client currentversion <projectname>\n");
        fprintf(stderr, "sample usage: ./client history <projectname>\n");
        fprintf(stderr, "sample usage: ./client checkout <projectname>\n");
        fprintf(stderr, "sample usage: ./client update <projectname>\n");
        fprintf(stderr, "sample usage: ./client upgrade <projectname>\n");
        fprintf(stderr, "sample usage: ./client commit <projectname>\n");
        fprintf(stderr, "sample usage: ./client push <projectname>\n");
        fprintf(stderr, "sample usage: ./client rollback <projectname>\n");
	// client only operation:
        fprintf(stderr, "sample usage: ./client add <projectname> <filename>\n");
        fprintf(stderr, "sample usage: ./client remove <projectname> <filename>\n");

        exit(EXIT_FAILURE);
    }

    // NOTE: this only needs to be done once, since the desired action is
    // to [cleanly] stop the program
    signal(SIGINT, sig_handler);

    // ** process client command "configure":
    //    place (hostname = ip) and (port number) in .config file
    //    "configure" is all on client, no need for sockfd to talk to server
    if (strcmp(argv[1],"configure") == 0) 
    {
	// argv[2] = ip, argv[3] = port
	if (process_configure(argv[2], argv[3]) != 0) {
		printf("\n** Error: process_configure()\n");
        	exit(EXIT_FAILURE);
	};
	return 0;
    }

    int fp = open("./.configure", O_RDWR);
    if (fp < 0){
        printf("File ./.configure does not exist. Run ./WTF configure first\n");
        exit(-1);
    }
        
    char input_string[100];
    char server_hostname[100];
    unsigned short server_port;
    char *token;
    
    // get IP address and port number from .configure file
    read(fp,input_string,100);
	
    // file is predefined
    token = strtok(input_string,":");
    strcpy(server_hostname,token);
    
    token = strtok(NULL,":");
    server_port = atoi(token);

    close(fp);

//fprintf(stderr, "\nserver_hostname=|%s|, server_port=|%d|\n", server_hostname, server_port);

    // get sockfd to talk to server
    //char *server_hostname = "127.0.0.1";
    //unsigned short server_port = 12345;
    sockfd = get_sockfd(server_hostname, server_port);
    if (sockfd == -1) {
        fprintf(stderr, "Error: get_sockfd(server_hostname, server_port)\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1],"create") == 0) // client command "create" 
    {
	// argv[2] = projectname
	if (process_create(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_create(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"checkout") == 0) // client command "checkout" 
    {
	// argv[2] = projectname
	if (process_checkout(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_checkout(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"commit") == 0) // client command "commit" 
    {
	// argv[2] = projectname
	if (process_commit(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_commit(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"push") == 0) // client command "push" 
    {
	// argv[2] = projectname
	if (process_push(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_push(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"add") == 0) // client command "add" 
    {
	// argv[2] = projectname
	if (process_add(sockfd, argv[2], argv[3]) != 0) {
		printf("\n** Error: process_add(sockfd, argv[2], argv[3]), argv[2]=|%s|, argv[3]=|%s|\n", 
			argv[2], argv[3]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"remove") == 0) // client command "remove" 
    {
	// argv[2] = projectname
	if (process_remove(sockfd, argv[2], argv[3]) != 0) {
		printf("\n** Error: process_remove(sockfd, argv[2], argv[3]), argv[2]=|%s|, argv[3]=|%s|\n", 
			argv[2], argv[3]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"currentversion") == 0) // client command "currentversion" 
    {
	// argv[2] = projectname
	if (process_currentversion(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_currentversion(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"history") == 0) // client command "history" 
    {
	// argv[2] = projectname
	if (process_history(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_history(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"rollback") == 0) // client command "rollback" 
    {
	// argv[2] = projectname, argv[3] = version
	if (process_rollback(sockfd, argv[2], argv[3]) != 0) {
		printf("\n** Error: process_rollback(sockfd, argv[2], argv[3]), argv[2]=|%s|, argv[3]=|%s|\n", 
			argv[2], argv[3]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"destroy") == 0) // client command "destroy" 
    {
	// argv[2] = projectname
	if (process_destroy(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_destroy(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"update") == 0) // client command "update"
    {
	// argv[2] = projectname
	if (process_update(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_update(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }
    else if (strcmp(argv[1],"upgrade") == 0) // client command "upgrade"
    {
	// argv[2] = projectname
	if (process_upgrade(sockfd, argv[2]) != 0) {
		printf("\n** Error: process_upgrade(sockfd, argv[2]), argv[2]=|%s|\n", 
			argv[2]);
    		close(sockfd);
        	exit(EXIT_FAILURE);
	};
    }

    // NOTE: we output this here when it's save to do so
    if (signo_taken)
        printf("\n** CTRL-C SIGINT captured by signal hander.\n\n");

    close(sockfd);
    exit(EXIT_SUCCESS);
}


int process_configure(char * ip, char * port)
{
	/*
        * Opens a text file for both reading and writing. It first 
        * truncates the file to zero length if it exists, otherwise 
        * creates a file if it does not exist.  */

        int filedesc = open(".configure", O_WRONLY | O_TRUNC | O_CREAT, 0744);

        if (filedesc<0){
                printf("could not open file");
                return 0;
        }

        char *address = (char*)malloc(100*sizeof(char));
        strcpy(address,ip);
        strcat(address,":");
        strcat(address,port);

        int writevalue = write(filedesc,address,100);
        if (writevalue<0){
                perror("There was an error writing to the .configure file");
                exit(-1);
        }

	close(filedesc);
	free(address);
        return 0;
};

int process_create(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char status[BUFSIZ];

   // check client project folder
   if (isDir(projectname)) {
	fprintf(stderr, "\nError: the project folder %s already exists in the client", projectname);
	return 1; 
   }

   // send client command "create" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "create");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'create' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }

   // receive project master .manifest from server
   // ** receive file:
   //    1) read file byte stream from sockfd (server) and 
   //    2) save the server path "Repository/projectName/master/.manifest"
   //       to the client "projectName/.manifest"
   //       ** receive1file(sockfd, "") will      
   //          1) remove "Repository/" and "master/" from the server path
   //          2) to form the client path "projectName/.manifest" to write
   if (receive1file(sockfd, "") != 0) 
   {
	fprintf(stderr, "\nError: return from receive1file(sockfd, "")\n");
	return 1;
   };
   fprintf(stderr, "\n** in client: create project completed, project=|%s|\n\n", proj_name);

   return 0;
}

int process_checkout(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char status[BUFSIZ];
   char path[BUFSIZ];
   char filePath_toSave[BUFSIZ];
   char cwd[BUFSIZ];

   // check client project folder
   if (isDir(projectname)) {
	fprintf(stderr, "\nError: the project folder %s already exists in the client", projectname);
	return 1; 
   }

   // send client command "checkout" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "checkout");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'checkout' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status = |%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status = |%s|\n\n", status);

   // ** receive file:
   //    1) read file byte stream from sockfd (server) and 
   //    2) save to filePath_toSave on (client)
   strcpy(filePath_toSave, "master.tar.bz2");
   if (receive1file(sockfd, filePath_toSave) != 0) 
   {
	fprintf(stderr, "\nError: receive1file(sockfd, filePath_toSave), filePath_toSave=|%s|\n",
		filePath_toSave);
	return 1;
   };
   fprintf(stderr, "\n** in client: received filePath_toSave=|%s|\n\n", filePath_toSave);

   // -------------------------------------------------------------------
   // unzip .tar.bz2 to create project folder
   // -------------------------------------------------------------------
   char commandLine[BUFSIZ];
   strcpy(commandLine, "rm -rf master");	// remove master folder if exist
   system(commandLine);

//   strcpy(commandLine, "tar -xvf master.tar.bz2");
   strcpy(commandLine, "tar -xf master.tar.bz2");
   fprintf(stderr, "\ncommandLine = |%s|\n", commandLine);    
   system(commandLine);

   rename("master", projectname);
   remove("master.tar.bz2");

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   return 0;
}

int process_update(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char status[BUFSIZ];
   struct LLnode * ptr;

   // send client command "update" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "update");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'update' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status=|%s|\n\n", status);

   //receive manifest
   char * path=(char *)malloc(50+sizeof(projectname));
   strcpy(path,projectname);
   strcat(path,"/.serverManifestForUpdate");
   receive1file(sockfd,path);
   
   // --------------------------------------------
   // copy from server manifest to linked list
   // --------------------------------------------
   struct LLnode * serverHead=(struct LLnode *)malloc(sizeof(struct LLnode));
   serverHead->path=NULL;
   serverHead->next = NULL;

printf("\npath=|%s|\n",path);

   int fileDesc=open(path, O_RDONLY, 0744);
   int fsize=fileSize(path);
   char* contents=(char*)malloc((fsize+20)*sizeof(char));
   contents[0]='\0';
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }

printf("\nfsize=|%d|\n",fsize);
contents[fsize]='\0';

   int i=0;
   int numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
   char projVersionNumServer[BUFSIZ];
   char * token=nextToken(contents,i,strlen(contents));//projNum
   i=i+strlen(token);
   strcpy(projVersionNumServer,token);
   free(token); // from nextToken() via malloc, free if no more use 

printf("\nprojVersionNumServer=|%s|\n",projVersionNumServer);

printf("\nstrlen(contents)=|%ld|, i=|%d|\n",strlen(contents),i);

   numTokens=0;
   ptr=serverHead;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));

// printf("\ntoken=|%s|\n",token);

	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			ptr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			ptr=ptr->next;
			ptr->next=NULL;
		}
		ptr->path=token;
		ptr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		ptr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		ptr->hash=token;
	}
	i=i+strlen(token);
	numTokens++;
   }
   close(fileDesc);
   free(contents);

   if (serverHead->path == NULL) {
	free(serverHead);
	serverHead = NULL;
   }

   ptr=serverHead;
   while (ptr != NULL) {
	printf("\nptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // copy from client manifest to linked list
   // ---------------------------------------------------------------
   struct LLnode * clientHead=(struct LLnode *)malloc(sizeof(struct LLnode));
   clientHead->path=NULL;
   clientHead->next = NULL;

   char * clientPath=(char *)malloc(50+sizeof(projectname));
   strcpy(clientPath,projectname);
   strcat(clientPath,"/.Manifest");

printf("\nclientPath=|%s|\n",clientPath);

   fileDesc=open(clientPath, O_RDONLY, 0744);
   fsize=fileSize(clientPath);
   contents=(char*)malloc((fsize+20)*sizeof(char));
   contents[0]='\0';
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }
   contents[fsize]='\0';

   i=0;
   char projVersionNumClient[BUFSIZ];
   token=nextToken(contents,i,strlen(contents));//projNum
   i=i+strlen(token);
   strcpy(projVersionNumClient,token);
   free(token); // from nextToken() via malloc, free if no more use

// printf("\nprojVersionNumClient=|%s|\n",projVersionNumClient);

   numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
   ptr=clientHead;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));
	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			ptr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			ptr=ptr->next;
			ptr->next=NULL;
		}
		ptr->path=token;
		ptr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		ptr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		ptr->hash=token;
	}
	i=i+strlen(token);
	numTokens++;
   }
   close(fileDesc);
   free(contents);

   if (clientHead->path == NULL) {
	free(clientHead);
	clientHead = NULL;
   }

   ptr=clientHead;
   while (ptr != NULL) {

	printf("\nptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // check project version
   // ---------------------------------------------------------------
   int i_projVersionNumClient = atoi(projVersionNumClient);
   int i_projVersionNumServer = atoi(projVersionNumServer);
   if (i_projVersionNumClient >= i_projVersionNumServer) {
	// send command execution status to server
	memset(status, 0, sizeof(status));
	strcpy(status, "serverProjectVersionNotHigher");
	send(sockfd, status, sizeof(status), 0);
	fprintf(stderr, "\nThe server project version is not higher than the client project version.\n");
	return 0;
   }
   fprintf(stderr, "\nprojVersionNumClient=|%s|, projVersionNumServer=|%s|\n",
		projVersionNumClient,projVersionNumServer);

   // ---------------------------------------------------------------
   // update all hashes in clientHead
   // ---------------------------------------------------------------
   ptr=clientHead;
   char tmp_path[BUFSIZ];
   while (ptr!=NULL) {
	strcpy(tmp_path, projectname);
	strcat(tmp_path, "/");
	strcat(tmp_path, ptr->path);
	char * newHash=getFileSHA1(tmp_path);
	if (strcmp(newHash,ptr->hash)!=0) {
		// update hash in clientHead 
		// do not increase file version # for now
		// will compare this with server's .Manifest
		// if change, write to .Commit with new file version #
		free(ptr->hash);
		ptr->hash=newHash;
	}
	ptr=ptr->next;
   }

   ptr=clientHead;
   while (ptr != NULL) {
	printf("\nptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // make a string of all updates needed
   // ---------------------------------------------------------------
   char * updateString=(char *)malloc(1);
   updateString[0]='\0';

   //from clientHead, find everything in serverHead
   ptr=clientHead;
   struct LLnode * searchNode;
   while (ptr!=NULL) {
	searchNode=LLsearch(serverHead,ptr->path);
	if (searchNode==NULL) {
		updateString=readjustBig(updateString,sizeof(ptr->path)+100);
		strcat(updateString,"D\t");
		char * tempPath=(char *)malloc(sizeof(ptr->path)+5);
		strcpy(tempPath,ptr->path);
		strcat(updateString,tempPath);
		strcat(updateString,"\t");
		strcat(updateString,ptr->vNum);
		strcat(updateString,"\t");
		strcat(updateString,ptr->hash);
		strcat(updateString,"\n");
		free(tempPath);
	}
	else if (strcmp(ptr->hash,searchNode->hash)!=0) {
		int server_file_version = atoi(searchNode->vNum);
		int client_file_version = atoi(ptr->vNum);
		if (client_file_version >= server_file_version) {
		   // send command execution status back to server
		   memset(status, 0, sizeof(status));
		   strcpy(status, "newHash_fileVersionNotHigherOnServer");
		   send(sockfd, status, sizeof(status), 0);
		   fprintf(stderr, "\nclient cannot update: NewHash_fileVersionNotHigherOnServer\n");
		   return 0;
		}
		updateString=readjustBig(updateString,sizeof(ptr->path)+100);
		strcat(updateString,"M\t");
		char * tempPath=(char *)malloc(sizeof(ptr->path)+5);
		strcpy(tempPath,ptr->path);
		strcat(updateString,tempPath);
		strcat(updateString,"\t");
		strcat(updateString,searchNode->vNum);
		strcat(updateString,"\t");
		strcat(updateString,searchNode->hash);
		strcat(updateString,"\n");
		free(tempPath);
 	}
	ptr = ptr->next;
   }

// printf("\nupdateString=|%s|\n",updateString);

   //from serverHead, find everything in clientHead
   ptr=serverHead;
//   struct LLnode * searchNode;
   while (ptr!=NULL) {
	searchNode=LLsearch(clientHead,ptr->path);
	if (searchNode==NULL) {
		updateString=readjustBig(updateString,sizeof(ptr->path)+100);
		strcat(updateString,"A\t");
		char * tempPath=(char *)malloc(sizeof(ptr->path)+5);
		strcpy(tempPath,ptr->path);
		strcat(updateString,tempPath);
		strcat(updateString,"\t");
		strcat(updateString,ptr->vNum);
		strcat(updateString,"\t");
		strcat(updateString,ptr->hash);
		strcat(updateString,"\n");
		free(tempPath);
	}
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // check updateString
   // ---------------------------------------------------------------
   fprintf(stderr, "\nupdateString=|%s|\n",updateString);

   // if nothing to commit, no need to generate .Update
   if (strlen(updateString) == 0) {
	char cPath[BUFSIZ];
	strcpy(cPath, projectname);
	strcat(cPath, "/.Update");
	remove(cPath);
	// send command execution status back to client
	memset(status, 0, sizeof(status));
	strcpy(status, "nothingToUpdate");
	send(sockfd, status, sizeof(status), 0);
	fprintf(stderr, "\nnothing to update\n\n");
	return 0;
   }

   // ---------------------------------------------------------------
   // make .Update file
   // ---------------------------------------------------------------
   char * clientUpdatePath=(char *)malloc(50+sizeof(projectname));
   strcpy(clientUpdatePath,projectname);
   strcat(clientUpdatePath,"/.Update");
   fileDesc=open(clientUpdatePath, O_WRONLY|O_TRUNC|O_CREAT, 0744);
   write(fileDesc, updateString, strlen(updateString));
   close(fileDesc);

   // send command execution status back to client
   memset(status, 0, sizeof(status));
   strcpy(status, "succeeded");
   send(sockfd, status, sizeof(status), 0);

   return 0;
}

int process_upgrade(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char status[BUFSIZ];

   //check if client has a .Update file
   char * pathU=(char *)malloc(50+sizeof(projectname));
   strcpy(pathU,projectname);
   strcat(pathU,"/.Update");
   if( (access( pathU, F_OK )) == -1 ) //I am not checking if it is empty or not
   {
	fprintf(stderr, "\nin client, the .Update file should be present\n\n"); 
	return 0;
   }
   free(pathU);

   // send client command "upgrade" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "upgrade");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'upgrade' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status=|%s|\n\n", status);

   //send command in .Update to server, transfer file if needed
   char pathUpdate[BUFSIZ];
   strcpy(pathUpdate,projectname);
   strcat(pathUpdate,"/.Update");
   int fileDesc=open(pathUpdate, O_RDONLY, 0744);
   int fsize=fileSize(pathUpdate);

   char* contents=(char*)malloc((fsize+50)*sizeof(char));
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }
   contents[fsize]='\0';

   int i=0;
   int numTokens=0; //action,tab,file path, newline
   char action;
   char * token;
   char filePath_toSave[BUFSIZ];
   numTokens=0;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));
	if (numTokens % 8==0) { //token is the action

printf("\naction token=|%s|\n",token);

		if (strcmp(token, "M")==0) {
			action='m';
			// sent 'Copy' to server
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, "M");
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent 'M' command to server.\n");
		}
		if (strcmp(token, "A")==0) {		
			action='a';
			// sent 'Add' to server (Add=Insert, "Add" used in spec)
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, "A");
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent 'A' command to server.\n");
		}
		if (strcmp(token,"D")==0) {
			action='d';
			// sent 'Remove' to server (Remove=Delete, "Remove" used in spec)
//   			memset(client_command, 0, sizeof(client_command));
//   			strcpy(client_command, "D");
//   			send(sockfd, client_command, sizeof(client_command), 0);
//   			fprintf(stderr, "\nin client, sent 'D' command to server.\n");
		}
	}
	if (numTokens % 8==2) { //token is the filepath

printf("\nfilepath token=|%s|\n",token);

		if (action=='m'||action=='a') {
			// sent 'filepath' to server 
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, token);
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent filepath |%s| to server.\n"
				, client_command);

			// ** receive file:
			strcpy(filePath_toSave,projectname);
			strcat(filePath_toSave,"/");
			strcat(filePath_toSave,token);
			if (receive1file(sockfd, filePath_toSave) != 0) // override if exist
			{
			  fprintf(stderr, 
			    "\nError: receive1file(sockfd, filePath_toSave), filePath_toSave=|%s|\n",
			    filePath_toSave);
			  return 1;
			};
			fprintf(stderr, "\n** in server: received filePath_toSave=|%s|\n", filePath_toSave);
		}
		if (action=='d') {
			// ** delete file:
			strcpy(filePath_toSave,projectname);
			strcat(filePath_toSave,"/");
			strcat(filePath_toSave,token);
			int ret = remove(filePath_toSave);
			if (ret == 0) {
				fprintf(stderr, "\n** in client: removed file=|%s|\n", filePath_toSave);
			}
			else {
				fprintf(stderr, "\n** warning: cannot remove file=|%s|\n", filePath_toSave);
			}
		}
	}
	i=i+strlen(token);
   	numTokens++;
   }
   close(fileDesc);
   free(contents);

   // send command execution status back to client
   memset(status, 0, sizeof(status));
   strcpy(status, "continue");
   send(sockfd, status, sizeof(status), 0);

   // client upgraded, get server .Manifest to override client .Manifest 
   strcpy(filePath_toSave,projectname);
   strcat(filePath_toSave,"/.Manifest");
   if (receive1file(sockfd, filePath_toSave) != 0) 
   {
	fprintf(stderr, "\nError: receive1file(sockfd, filePath_toSave), filePath_toSave=|%s|\n",
		filePath_toSave);
	return 1;
   };
   fprintf(stderr, "\n** in client: received filePath_toSave=|%s|\n\n", filePath_toSave);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status=|%s|\n\n", status);

   // remove .Update after apply 
   char cPath[BUFSIZ];
   char cPath2[BUFSIZ];
   strcpy(cPath, projectname);
   strcat(cPath, "/.Update");
   strcpy(cPath2, projectname);
   strcat(cPath2, "/.Update_archive");
   rename(cPath,cPath2);

   return 0;
}

int process_commit(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char status[BUFSIZ];
   struct LLnode * ptr;

   //check if client has a .Update file
   char * pathU=(char *)malloc(15+sizeof(projectname));
   strcpy(pathU,projectname);
   strcat(pathU,"/.Update");
   if( (access( pathU, F_OK )) != -1 ) //I am not checking if it is empty or not
   {
	fprintf(stderr, "\nin client, the .Update file should not be present"); 
	return 0;
   }

   // send client command "commit" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "commit");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'commit' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   //receive manifest
   char * path=(char *)malloc(50+sizeof(projectname));
   strcpy(path,projectname);
   strcat(path,"/.serverManifest");
   receive1file(sockfd,path);
   
   // ---------------------------------------------------------------
   // copy from server manifest to linked list
   // ---------------------------------------------------------------
   struct LLnode * serverHead=(struct LLnode *)malloc(sizeof(struct LLnode));
   serverHead->path=NULL;
   serverHead->next = NULL;

printf("\npath=|%s|\n",path);

   int fileDesc=open(path, O_RDONLY, 0744);
   int fsize=fileSize(path);
   char* contents=(char*)malloc((fsize+20)*sizeof(char));
   contents[0]='\0';
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }

printf("\nfsize=|%d|\n",fsize);
contents[fsize]='\0';

   int i=0;
   int numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
   char projVersionNumServer[BUFSIZ];
   char * token=nextToken(contents,i,strlen(contents));//projNum
   i=i+strlen(token);
   strcpy(projVersionNumServer,token);
   free(token); // from nextToken() via malloc, free if no more use 

printf("\nprojVersionNumServer=|%s|\n",projVersionNumServer);

printf("\nstrlen(contents)=|%ld|, i=|%d|\n",strlen(contents),i);

   numTokens=0;
   ptr=serverHead;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));

// printf("\ntoken=|%s|\n",token);

	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			ptr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			ptr=ptr->next;
			ptr->next=NULL;
		}
		ptr->path=token;
		ptr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		ptr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		ptr->hash=token;
	}
	i=i+strlen(token);
	numTokens++;
   }
   close(fileDesc);
   free(contents);

   if (serverHead->path == NULL) {
	free(serverHead);
	serverHead = NULL;
   }

   ptr=serverHead;
   while (ptr != NULL) {
	printf("\nptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // copy from client manifest to linked list
   // ---------------------------------------------------------------
   struct LLnode * clientHead=(struct LLnode *)malloc(sizeof(struct LLnode));
   clientHead->path=NULL;
   clientHead->next = NULL;

   char * clientPath=(char *)malloc(50+sizeof(projectname));
   strcpy(clientPath,projectname);
   strcat(clientPath,"/.Manifest");

printf("\nclientPath=|%s|\n",clientPath);

   fileDesc=open(clientPath, O_RDONLY, 0744);
   fsize=fileSize(clientPath);
   contents=(char*)malloc((fsize+20)*sizeof(char));
   contents[0]='\0';
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }
   contents[fsize]='\0';

   i=0;
   char projVersionNumClient[BUFSIZ];
   token=nextToken(contents,i,strlen(contents));//projNum
   i=i+strlen(token);
   strcpy(projVersionNumClient,token);
   free(token); // from nextToken() via malloc, free if no more use

// printf("\nprojVersionNumClient=|%s|\n",projVersionNumClient);

   numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
   ptr=clientHead;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));
	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			ptr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			ptr=ptr->next;
			ptr->next=NULL;
		}
		ptr->path=token;
		ptr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		ptr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		ptr->hash=token;
	}
	i=i+strlen(token);
	numTokens++;
   }
   close(fileDesc);
   free(contents);

   if (clientHead->path == NULL) {
	free(clientHead);
	clientHead = NULL;
   }

   ptr=clientHead;
   while (ptr != NULL) {
	printf("\nptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // check project version
   // ---------------------------------------------------------------
   if (strcmp(projVersionNumClient,projVersionNumServer)!=0) {
	// send command execution status to server
	memset(status, 0, sizeof(status));
	strcpy(status, "notUpdatingMostRecentVersionOnServer");
	send(sockfd, status, sizeof(status), 0);
	fprintf(stderr, "\nthe client is not updating the most recent version on the server\n");
	return 0;
   }
   fprintf(stderr, "\nMatched: projVersionNumClient=|%s|, projVersionNumServer=|%s|\n",
		projVersionNumClient,projVersionNumServer);

   // ---------------------------------------------------------------
   // update all hashes in clientHead
   // ---------------------------------------------------------------
   ptr=clientHead;
   char tmp_path[BUFSIZ];
   while (ptr!=NULL) {
	strcpy(tmp_path, projectname);
	strcat(tmp_path, "/");
	strcat(tmp_path, ptr->path);
	char * newHash=getFileSHA1(tmp_path);
	if (strcmp(newHash,ptr->hash)!=0) {
		// update hash in clientHead 
		// do not increase file version # for now
		// will compare this with server's .Manifest
		// if change, write to .Commit with new file version #
		free(ptr->hash);
		ptr->hash=newHash;
	}
	ptr=ptr->next;
   }

   ptr=clientHead;
   while (ptr != NULL) {
	printf("\nptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // make a string of all updates needed
   // ---------------------------------------------------------------
   char * commitString=(char *)malloc(1);
   commitString[0]='\0';

   //from serverHead, find everything in clientHead
   ptr=serverHead;
   struct LLnode * searchNode;
   while (ptr!=NULL) {
	searchNode=LLsearch(clientHead,ptr->path);
	if (searchNode==NULL) {
		commitString=readjustBig(commitString,sizeof(ptr->path)+100);
		strcat(commitString,"Remove\t");
		char * tempPath=(char *)malloc(sizeof(ptr->path)+5);
		strcpy(tempPath,ptr->path);
		strcat(commitString,tempPath);
		strcat(commitString,"\t");
		strcat(commitString,ptr->vNum);
		strcat(commitString,"\t");
		strcat(commitString,ptr->hash);
		strcat(commitString,"\n");
		free(tempPath);
	}
	else if (strcmp(ptr->hash,searchNode->hash)!=0) {
		int client_file_version = atoi(searchNode->vNum);
		int client_file_new_version = client_file_version + 1;
		char str_client_file_new_version[BUFSIZ];
		itoa(client_file_new_version, str_client_file_new_version);
		int server_file_version = atoi(ptr->vNum);
		if (server_file_version >= (client_file_version + 1)) {
		   // send command execution status back to server
		   memset(status, 0, sizeof(status));
		   strcpy(status, "fileVersionHigherOnServer");
		   send(sockfd, status, sizeof(status), 0);
		   fprintf(stderr, "\nthe client cannot commit: fileVersionHigherOnServer\n");
		   return 0;
		}
		commitString=readjustBig(commitString,sizeof(ptr->path)+100);
		strcat(commitString,"Copy\t");
		char * tempPath=(char *)malloc(sizeof(ptr->path)+5);
		strcpy(tempPath,ptr->path);
		strcat(commitString,tempPath);
		strcat(commitString,"\t");
		strcat(commitString,str_client_file_new_version);
		strcat(commitString,"\t");
		strcat(commitString,searchNode->hash);
		strcat(commitString,"\n");
		free(tempPath);
 	}
	ptr = ptr->next;
   }

// printf("\ncommitString=|%s|\n",commitString);

   //from clientHead, find everything in serverHead
   ptr=clientHead;
//    struct LLnode * searchNode;
   while (ptr!=NULL) {
	searchNode=LLsearch(serverHead,ptr->path);
	if (searchNode==NULL) {
		commitString=readjustBig(commitString,sizeof(ptr->path)+100);
		strcat(commitString,"Add\t");
		char * tempPath=(char *)malloc(sizeof(ptr->path)+5);
		strcpy(tempPath,ptr->path);
		strcat(commitString,tempPath);
		strcat(commitString,"\t");
		strcat(commitString,"1");
		strcat(commitString,"\t");
		strcat(commitString,ptr->hash);
		strcat(commitString,"\n");
		free(tempPath);
	}
	ptr = ptr->next;
   }

   // ---------------------------------------------------------------
   // check commitString
   // ---------------------------------------------------------------
   fprintf(stderr, "\ncommitString=|%s|\n",commitString);

   // if nothing to commit, no need to generate .Commit
   if (strlen(commitString) > 0) {
	// send command execution status back to client
	memset(status, 0, sizeof(status));
	strcpy(status, "continue");
	send(sockfd, status, sizeof(status), 0);
   }
   else {
	char cPath[BUFSIZ];
	strcpy(cPath, projectname);
	strcat(cPath, "/.Commit");
	remove(cPath);
	// send command execution status back to client
	memset(status, 0, sizeof(status));
	strcpy(status, "nothingToCommit");
	send(sockfd, status, sizeof(status), 0);
	fprintf(stderr, "\nnothing to commit\n\n");
	return 0;
   }

   // ---------------------------------------------------------------
   // make .Commit file
   // ---------------------------------------------------------------
   char * clientCommitPath=(char *)malloc(15+sizeof(projectname));
   strcpy(clientCommitPath,projectname);
   strcat(clientCommitPath,"/.Commit");
   fileDesc=open(clientCommitPath, O_WRONLY|O_TRUNC|O_CREAT, 0744);
   write(fileDesc, commitString, strlen(commitString));
   close(fileDesc);

   // ---------------------------------------------------------------
   // ** send file:
   //    1) read byte stream from "filePath" on (client), and 
   //    2) write to "sockfd" (server)
   // ---------------------------------------------------------------
   if (send1file(sockfd, clientCommitPath) != 0) {
	fprintf(stderr, "\nError: send1file(int sockfd, char * file_path), file_path=|%s|\n",
		clientCommitPath);
	return 1;
   };
   fprintf(stderr, "\n** in client: sent file_path=|%s|\n", clientCommitPath);

   // ---------------------------------------------------------------
   // receive status from server
   // ---------------------------------------------------------------
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   return 0;
}

int process_push(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char status[BUFSIZ];

   //check if client has a .Update file
   char * pathU=(char *)malloc(50+sizeof(projectname));
   strcpy(pathU,projectname);
   strcat(pathU,"/.Update");
   if( (access( pathU, F_OK )) != -1 ) //I am not checking if it is empty or not
   {
	fprintf(stderr, "\nin client, the .Update file should not be present"); 
	return 0;
   }
   free(pathU);

   //check if client has a .Commit file
   char * pathC=(char *)malloc(50+sizeof(projectname));
   strcpy(pathC,projectname);
   strcat(pathC,"/.Commit");
   if( (access( pathU, F_OK )) == -1 ) //need .Commit
   {
	fprintf(stderr, "\nin client, the .Commit file should be present\n\n"); 
	return 0;
   }
   free(pathC);

   // send client command "push" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "push");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'push' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   // ** send file:
   //    1) read byte stream from "filePath" on (client), and 
   //    2) write to "sockfd" (server)
   char pathCommit[BUFSIZ];
   strcpy(pathCommit,projectname);
   strcat(pathCommit,"/.Commit");
   if (send1file(sockfd, pathCommit) != 0) {
	fprintf(stderr, "\nError: send1file(int sockfd, char * file_path), file_path=|%s|\n",
		pathCommit);
	return 1;
   };
   fprintf(stderr, "\n** in client: sent file_path=|%s|\n", pathCommit);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   //send command in .Commit to server, transfer file if needed
   strcpy(pathCommit,projectname);
   strcat(pathCommit,"/.Commit");
   int fileDesc=open(pathCommit, O_RDONLY, 0744);
   int fsize=fileSize(pathCommit);

   char* contents=(char*)malloc((fsize+20)*sizeof(char));
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }
   contents[fsize]='\0';

   int i=0;
   int numTokens=0; //action,tab,file path, newline
   char action;
   char * token;
   numTokens=0;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));
	if (numTokens % 8==0) { //token is the action

printf("\naction token=|%s|\n",token);

		if (strcmp(token, "Copy")==0) {
			action='c';
			// sent 'Copy' to server
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, "Copy");
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent 'Copy' to server.\n");
		}
		if (strcmp(token, "Add")==0) {		
			action='a';
			// sent 'Add' to server (Add=Insert, "Add" used in spec)
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, "Add");
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent 'Add' to server.\n");
		}
		if (strcmp(token,"Remove")==0) {
			action='r';
			// sent 'Remove' to server (Remove=Delete, "Remove" used in spec)
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, "Remove");
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent 'Remove' to server.\n");
		}
	}
	if (numTokens % 8==2) { //token is the filepath

printf("\nfilepath token=|%s|\n",token);

		if (action=='c'||action=='a') {
			// sent 'filepath' to server 
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, token);
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent filepath |%s| to server.\n"
				, client_command);

			// ** send file:
			strcpy(pathCommit,projectname);
			strcat(pathCommit,"/");
			strcat(pathCommit,token);
			if (send1file(sockfd, pathCommit) != 0) {
			  fprintf(stderr, "\nError: send1file(sockfd, file_path), file_path=|%s|\n",
					pathCommit);
			  return 1;
			};
			fprintf(stderr, "\n** in client: sent file_path=|%s|\n", pathCommit);
		}
		if (action=='r') {
			// sent 'filepath' to server 
   			memset(client_command, 0, sizeof(client_command));
   			strcpy(client_command, token);
   			send(sockfd, client_command, sizeof(client_command), 0);
   			fprintf(stderr, "\nin client, sent filepath |%s| to server.\n"
				, client_command);
		}
	}
	i=i+strlen(token);
   	numTokens++;
   }
   close(fileDesc);
   free(contents);

   // send command execution status back to client
   memset(status, 0, sizeof(status));
   strcpy(status, "succeeded");
   send(sockfd, status, sizeof(status), 0);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   // ** "Push" succeeded, .Commit applied, server project version advanced
   //    now get the updated server .Manifest, to override the client .Manifest 
   char filePath_toSave[BUFSIZ];
   strcpy(filePath_toSave,projectname);
   strcat(filePath_toSave,"/.Manifest");
   if (receive1file(sockfd, filePath_toSave) != 0) 
   {
	fprintf(stderr, "\nError: receive1file(sockfd, filePath_toSave), filePath_toSave=|%s|\n",
		filePath_toSave);
	return 1;
   };
   fprintf(stderr, "\n** in client: received filePath_toSave=|%s|\n\n", filePath_toSave);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   // remove .Commit after apply 
   char cPath[BUFSIZ];
   strcpy(cPath, projectname);
   strcat(cPath, "/.Commit");
   remove(cPath);

   return 0;
}

int process_destroy(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char versionNum[BUFSIZ];
   char status[BUFSIZ];

   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "destroy");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'destroy' to server.\n");

   //send project name to server
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status = |%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status = |%s|\n\n", status);

   return 0;
}

int process_currentversion(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char filePath_toSave[BUFSIZ];
   char status[BUFSIZ];

   // send client command "currentversion" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "currentversion");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'currentversion' to server.\n");

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   strcpy(filePath_toSave, projectname);
   strcat(filePath_toSave, "/.tmp_Manifest");
   if (receive1file(sockfd, filePath_toSave) != 0) 
   {
	fprintf(stderr, "\nError: receive1file(sockfd, filePath_toSave), filePath_toSave=|%s|\n",
		filePath_toSave);
	return 1;
   };
   fprintf(stderr, "\n** in client: received filePath_toSave=|%s|\n\n", filePath_toSave);

   fprintf(stderr, "\n********************************************");
   fprintf(stderr, "\n** Project Name: %s", projectname);
   fprintf(stderr, "\n--------------------------------------------\n");
   print_current_manifest(filePath_toSave);
   fprintf(stderr, "********************************************\n\n");

   return 0;
}

int process_history(int sockfd, char * projectname)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char filePath_toSave[BUFSIZ];
   char status[BUFSIZ];

   // send client command "history" to server,
   // to guide the server to run the designated function 
   // to communicate with the client, to accompish the mission  
   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "history");
   send(sockfd, client_command, sizeof(client_command), 0);

   // send "projectname" to server   
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status. status=|%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status. status=|%s|\n\n", status);

   // ** receive file:
   strcpy(filePath_toSave, projectname);
   strcat(filePath_toSave, "/.tmp_History");
   if (receive1file(sockfd, filePath_toSave) != 0) 
   {
	fprintf(stderr, "\nError: receive1file(sockfd, filePath_toSave), filePath_toSave=|%s|\n",
		filePath_toSave);
	return 1;
   };
   fprintf(stderr, "\n** in client: received filePath_toSave=|%s|\n\n", filePath_toSave);

   // display .tmp_History, filePath_toSave
   strcpy(client_command, "cat ");
   strcat(client_command, filePath_toSave);
   strcat(client_command, " | more");
   system(client_command);

   return 0;
}

int process_add(int sockfd, char * projectname, char * filename)
{
   // return 1 :  file
   // return 0 :  directory
   // return -1 : invalid name
   if (isFile(projectname) != 0) {
	fprintf(stderr, "\n** project does not exist, projectname=|%s|\n\n", 
		projectname);
	return 1;
   }

   char tmpPath[BUFSIZ];
   strcpy(tmpPath, projectname);
   strcat(tmpPath, "/");
   strcat(tmpPath, filename);
   // return 1 :  file
   // return 0 :  directory
   // return -1 : invalid name
   if (isFile(tmpPath) != 1) {
	fprintf(stderr, "\n** file does not exist, filePath=|%s|", 
		tmpPath);
	fprintf(stderr, "\n** make sure to include subdir in input filename under project folder |%s|",
		projectname);
	fprintf(stderr, "\n** example: subdir1/subdir2/data.txt\n\n");
	return 1;
   }

   // --------------------------------------------
   // copy from client manifest to linked list
   // --------------------------------------------
   struct LLnode * clientHead=(struct LLnode *)malloc(sizeof(struct LLnode));
   clientHead->path = NULL;
   clientHead->next = NULL;

   char * clientPath=(char *)malloc(45*sizeof(char)+sizeof(projectname));
   strcpy(clientPath,projectname);
   strcat(clientPath,"/.Manifest");

printf("\nclientPath=|%s|\n",clientPath);

   int fileDesc=open(clientPath, O_RDONLY, 0744);
   int fsize=fileSize(clientPath);
   char * contents=(char*)malloc((fsize+20)*sizeof(char));
   contents[0]='\0';
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }
   contents[fsize]='\0';

   int i=0;
   char projVersionNumClient[BUFSIZ];
   char * token=nextToken(contents,i,strlen(contents));//projNum
   i=i+strlen(token);
   strcpy(projVersionNumClient,token);
   free(token); // from nextToken() via malloc, free if no more use

printf("\nprojVersionNumClient=|%s|\n",projVersionNumClient);

   int numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
   struct LLnode * ptr;
   ptr=clientHead;

printf("\nstrlen(contents)=|%ld|\n",strlen(contents));

   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));
	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			ptr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			ptr=ptr->next;
			ptr->next=NULL;
		}
		ptr->path=token;
		ptr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		ptr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		ptr->hash=token;
	}
	i=i+strlen(token);
	numTokens++;
   }
   close(fileDesc);
   free(contents);

   if (clientHead->path == NULL) {
	free(clientHead);
	clientHead = NULL;
   }

   printf("\n.Manifest before add\n");
   ptr=clientHead;
   while (ptr != NULL) {
	printf("ptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   //from clientHead, find input filename, if not, then "add"
   struct LLnode * searchNode;
   searchNode=LLsearch(clientHead,filename);
   if (searchNode==NULL) {
	// update linked list (.Manifest)
	char * hash=getFileSHA1(tmpPath);
    	char * tmpPath2=(char*)malloc((100)*sizeof(char));
	strcpy(tmpPath2, filename); 
	clientHead=LLinsert(clientHead, tmpPath2, "1", hash);
   }
   else {
	fprintf(stderr, "\n** Warning: cannot add, file already in .Manifest, filename=|%s|\n\n", 
		filename);
	return 0;
   }

   printf("\n.Manifest after add\n");
   ptr=clientHead;
   while (ptr != NULL) {
	printf("ptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   //copy the manifest linked list to the manifest file
   char * s=(char *)malloc(20);
   s[0]='\0';

   strcpy(s,projVersionNumClient);
   strcat(s,"\n");
   struct LLnode * node_ptr=clientHead;
   while (node_ptr!=NULL) {
	int size=strlen(node_ptr->path)+strlen(node_ptr->hash)+strlen(node_ptr->vNum)+10;
	s=readjustBig(s,size); //this is the function from Asst2, not in utils.c yet
	strcat(s,node_ptr->path);
	strcat(s,"\t");
	strcat(s,node_ptr->vNum);
	strcat(s,"\t");
	strcat(s,node_ptr->hash);
	strcat(s,"\n");
	node_ptr=node_ptr->next;
   }
   // write to file
   char manifestPath[BUFSIZ];
   strcpy(manifestPath,projectname);
   strcat(manifestPath,"/.Manifest");
   fileDesc=open(manifestPath, O_WRONLY|O_TRUNC|O_CREAT, 0744);
   write(fileDesc, s, strlen(s));
   close(fileDesc);
   free(s);

   return 0;
}


int process_remove(int sockfd, char * projectname, char * filename)
{
   // return 1 :  file
   // return 0 :  directory
   // return -1 : invalid name
   if (isFile(projectname) != 0) {
	fprintf(stderr, "\n** project does not exist, projectname=|%s|\n", 
		projectname);
	return 1;
   }

   char tmpPath[BUFSIZ];
   strcpy(tmpPath, projectname);
   strcat(tmpPath, "/");
   strcat(tmpPath, filename);
   // return 1 :  file
   // return 0 :  directory
   // return -1 : invalid name
   if (isFile(tmpPath) == 1) {
	fprintf(stderr, "\n** information: file exist, filePath=|%s|", 
		tmpPath);
        fprintf(stderr, "\n** make sure to include subdir in input filename under project folder |%s|",
		projectname);
	fprintf(stderr, "\n** example: subdir1/subdir2/data.txt\n\n");
   }
   else {
	fprintf(stderr, "\n** information: file does exist, filePath=|%s|", 
		tmpPath);
        fprintf(stderr, "\n** make sure to include subdir in input filename under project folder |%s|",
		projectname);
	fprintf(stderr, "\n** example: subdir1/subdir2/data.txt\n\n");
   }

   // --------------------------------------------
   // copy from client manifest to linked list
   // --------------------------------------------
   struct LLnode * clientHead=(struct LLnode *)malloc(sizeof(struct LLnode));
   clientHead->path = NULL;
   clientHead->next = NULL;

   char * clientPath=(char *)malloc(45+sizeof(projectname));
   strcpy(clientPath,projectname);
   strcat(clientPath,"/.Manifest");

printf("\nclientPath=|%s|\n",clientPath);

   int fileDesc=open(clientPath, O_RDONLY, 0744);
   int fsize=fileSize(clientPath);
   char * contents=(char*)malloc((fsize+20)*sizeof(char));
   contents[0]='\0';
   if (read(fileDesc, contents, fsize)==0) { //error
	return 1;
   }
   contents[fsize]='\0';

   int i=0;
   char projVersionNumClient[BUFSIZ];
   char * token=nextToken(contents,i,strlen(contents));//projNum
   i=i+strlen(token);
   strcpy(projVersionNumClient,token);
   free(token); // from nextToken() via malloc, free if no more use

   int numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
   struct LLnode * ptr;
   ptr=clientHead;
   while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));
	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			ptr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			ptr=ptr->next;
			ptr->next=NULL;
		}
		ptr->path=token;
		ptr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		ptr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		ptr->hash=token;
	}
	i=i+strlen(token);
	numTokens++;
   }
   close(fileDesc);
   free(contents);

   if (clientHead->path == NULL) {
	free(clientHead);
	clientHead = NULL;
   }

   printf("\n.Manifest before remove\n");
   ptr=clientHead;
   while (ptr != NULL) {
	printf("ptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   //from clientHead, find input filename, if found, then "remove"
   struct LLnode * searchNode;
   searchNode=LLsearch(clientHead,filename);
   if (searchNode==NULL) {
	fprintf(stderr, "\n** Warning: cannot remove, filename=|%s| not in .Manifest\n\n", 
		filename);
	return 0;
   }
   else {
	clientHead=LLdelete(clientHead, filename);
   }

   printf("\n.Manifest after remove\n");
   ptr=clientHead;
   while (ptr != NULL) {
	printf("ptr->path=|%s|,ptr->vNum=|%s|,ptr->hash=|%s|\n",
		ptr->path,ptr->vNum,ptr->hash);
	ptr = ptr->next;
   }

   //copy the manifest linked list to the manifest file
   char * s=(char *)malloc(20);
   s[0]='\0';

   strcpy(s,projVersionNumClient);
   strcat(s,"\n");
   struct LLnode * node_ptr=clientHead;
   while (node_ptr!=NULL) {
	int size=strlen(node_ptr->path)+strlen(node_ptr->hash)+strlen(node_ptr->vNum)+10;
	s=readjustBig(s,size); //this is the function from Asst2, not in utils.c yet
	strcat(s,node_ptr->path);
	strcat(s,"\t");
	strcat(s,node_ptr->vNum);
	strcat(s,"\t");
	strcat(s,node_ptr->hash);
	strcat(s,"\n");
	node_ptr=node_ptr->next;
   }
   // write to file
   char manifestPath[BUFSIZ];
   strcpy(manifestPath,projectname);
   strcat(manifestPath,"/.Manifest");
   fileDesc=open(manifestPath, O_WRONLY|O_TRUNC|O_CREAT, 0744);
   write(fileDesc, s, strlen(s));
   close(fileDesc);
   free(s);

   return 0;
}


int process_rollback(int sockfd, char * projectname, char * versionNumber)
{
   char client_command[BUFSIZ];
   char proj_name[BUFSIZ];
   char versionNum[BUFSIZ];
   char status[BUFSIZ];

   memset(client_command, 0, sizeof(client_command));
   strcpy(client_command, "rollback");
   send(sockfd, client_command, sizeof(client_command), 0);
   fprintf(stderr, "\nin client, sent client command 'rollback' to server.\n");

   //send project name to server
   memset(proj_name, 0, sizeof(proj_name));
   strcpy(proj_name, projectname);
   send(sockfd, proj_name, sizeof(proj_name), 0);
   fprintf(stderr, "\nin client, sent proj_name |%s| to server.\n", proj_name);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received server execution status = |%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status = |%s|\n\n", status);

   //send version Number to server
   memset(versionNum, 0, sizeof(versionNumber));
   strcpy(versionNum, versionNumber);
   send(sockfd, versionNum, sizeof(versionNum), 0);

   // receive status from server
   recv(sockfd, status, sizeof(status), 0);
   if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received server execution status = |%s|\n\n", status);
	return 1;
   }
   fprintf(stderr, "\nin client, received server execution status = |%s|\n\n", status);

   return 0;
}

int print_current_manifest(char *filePath){
        struct stat sb;
        lstat(filePath,&sb);
        int size;
        size = sb.st_size;
        char fileData[size];

        //open and read file
        int fp = open(filePath,O_RDWR);
        read(fp,fileData,size);

        //tokenize and parse through file data
        char *token;
        char *LineData;
        const char delim1[2] = "\n";
        const char delim2[2] = "\t";
        char *saveptr1;
        char *saveptr2;

        token = strtok_r(fileData,delim1,&saveptr1);

	printf("Project version number: %s\n", token);
        while((token = strtok_r(NULL,delim1,&saveptr1)) != NULL){
                LineData = strtok_r(token,delim2,&saveptr2);
                printf("File Name: %s\t",LineData);
                LineData = strtok_r(NULL,delim2,&saveptr2);
                printf("Version Number: %s\n",LineData);
        }
        return 0;
}





