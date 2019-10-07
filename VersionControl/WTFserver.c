// ================================================
// server.c 
// = interface
// ./executable [<port>]
//	./server 12345
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

#include "WTFserver.h"

#define MAX_MUTEXLOCK 1000

struct LLnode {
	char * path;
	char * vNum;
	char * hash;
//	char * projName;
	struct LLnode* next;
};

// NOTE: this consolidates four arrays that were indexed by socket_index
struct client {
    socklen_t client_len;
    struct sockaddr_in client_address;
    int client_sockfd;
    pthread_t thread;
};

enum { PORTSIZE = 6 };


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
void reverse(char s[]);
void itoa(int n, char s[]);
int isFile(const char* name);
const char *getUserName();
char* readFileDatafromFileName(char *filePath);
char* createFilePath(char* projectname,char* fileName);
int create_file(char * logPath);
void removingDirectory(char* projectPath);
int init_mutexlock();
int mutex_lock(char * projectname);
int mutex_unlock(char * projectname, int lock_index);

struct LLnode * LLsearch(struct LLnode * head, char * word);
struct LLnode * LLinsert(struct LLnode * head, char * path, char * vNum, char * hash);
struct LLnode * LLdelete(struct LLnode * head, char * path);

void * forClient(void *ptr);

int process_create(void *ptr);
int process_destroy(void *ptr);
int process_currentversion(void *ptr);
int process_history(void *ptr);
int process_checkout(void *ptr);
int process_update(void *ptr);
int process_upgrade(void *ptr);
int process_commit(void *ptr);
int process_push(void *ptr);
int process_rollback(void *ptr);

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("\n** CTRL-C SIGINT captured by signal hander.\n\n");
    }
}

/*
struct mutexlock * head_pLock = NULL;
*/

pthread_mutex_t pLock_mutex[MAX_MUTEXLOCK];
char pLock_name[MAX_MUTEXLOCK][BUFSIZ];

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int enable = 1;
    int server_sockfd;
    unsigned short server_port = 12345u;
    char portNum[PORTSIZE];

    // NOTE: now all client related data is malloc'ed
    struct client *ctl;

    if (argc != 2) {
        fprintf(stderr, "Usage   ./server  <port>\n");
        exit(EXIT_FAILURE);
    }
    server_port = strtol(argv[1], NULL, 10);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // ipv4
    hints.ai_socktype = SOCK_STREAM;    // tcp
    hints.ai_flags = AI_PASSIVE;        // fill in my IP for me

    sprintf(portNum, "%d", server_port);
    getaddrinfo(NULL, portNum, &hints, &res);

    server_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    if (bind(server_sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sockfd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "listening on port %d\n", server_port);

    // NOTE: let the threads to run detached so no need to wait
    // for them to do cleanup -- the thread now does its own close/cleanup
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,1);

    // NOTE: this only needs to be done once
    signal(SIGINT, sig_handler);

    // init mutex
    if (init_mutexlock() != 0) {
	perror("\n** Error: mutex init has failed\n");
        exit(EXIT_FAILURE);
    };
    fprintf(stderr, "\n** in server, mutex initialized\n\n");

    while (1) {
        
        ctl = malloc(sizeof(struct client));
        if (ctl == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        ctl->client_len = sizeof(ctl->client_address);
        puts("waiting for client");

        ctl->client_sockfd = accept(server_sockfd,
            (struct sockaddr *) &ctl->client_address, &ctl->client_len);

        if (ctl->client_sockfd < 0) {
            perror("Cannot accept connection\n");
            close(server_sockfd);
            exit(EXIT_FAILURE);
        }
        
        pthread_create(&ctl->thread, &attr, forClient, ctl);

    }

    return EXIT_SUCCESS;
}

void * forClient(void *ptr)
{
    struct client *ctl = ptr;
    int connect_socket = ctl->client_sockfd;
    char clientCommand[BUFSIZ];

    // Thread number means client's id
    printf("Thread number %ld\n", pthread_self());

    // =============================================================
    // ** client send "client command" (a simple string) to server
    //    to indicate which "client command" need to be served,
    //    so the server can invoke the right function 
    //    to communicate with the client, to accomplish the mission.
    // ** not all "client command" involve server actions
    // ** "client command" involve server action are
    //    1) create 2) destroy 
    //    3) currentversion 4) history 
    //    5) checkout 
    //    6) update 7) upgrade 8) commit 9) push 
    //    10) rollback
    // ** "client command" does not involve server action are:
    //    ("client command" take action only on client side) 
    //    1) configure 2) add 3) remove 
    // ** "client command": send1file2server 
    //	  is added as a sample to show how the work is completed 
    //    through client-server communication 
    // =============================================================
    while (recv(connect_socket, clientCommand, sizeof(clientCommand), 0)) {
	if (strcmp(clientCommand, "create") == 0) {
		process_create(ptr);
	}
	else if (strcmp(clientCommand, "destroy") == 0) {
		process_destroy(ptr);
	}
	else if (strcmp(clientCommand, "currentversion") == 0) {
		process_currentversion(ptr);
	}
	else if (strcmp(clientCommand, "history") == 0) {
		process_history(ptr);
	}
	else if (strcmp(clientCommand, "checkout") == 0) {
		process_checkout(ptr);
	}
	else if (strcmp(clientCommand, "update") == 0) {
		process_update(ptr);
	}
	else if (strcmp(clientCommand, "upgrade") == 0) {
		process_upgrade(ptr);
	}
	else if (strcmp(clientCommand, "commit") == 0) {
		process_commit(ptr);
	}
	else if (strcmp(clientCommand, "push") == 0) {
		process_push(ptr);
	}
	else if (strcmp(clientCommand, "rollback") == 0) {
		process_rollback(ptr);
	}
	else {
    		fprintf(stderr, "\ninvalid client command = |%s|\n", clientCommand);
	}
    }

    fprintf(stderr, "Client dropped connection\n");

    // NOTE: do all client related cleanup here
    close(connect_socket);
    free(ctl);

    return (void *) 0;
}

int process_create(void *client_ptr) 
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'create' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);

    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    // implement "create projectname" 
    // 1) check if the project folder exists 
    // 2) if pre-exist, send message back to client "project_pre-exist"
    // 3) if not, create project folder path and .Manifest file: 
    // 4) send project master .Manifest back to client 
    // 5) use "ls -l -a" to see file .Manifest 

    char * path=(char *)malloc(50+sizeof(projectname));
    strcpy(path,"Repository/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(path,projectname);

    if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO)==-1) {
    	if (errno==EEXIST) { 
    		// send command execution status back to client
    		memset(status, 0, sizeof(status));
    		strcpy(status, "ProjectAlreadyExists");
    		send(connect_socket, status, sizeof(status), 0);
		// print message from server console
		fprintf(stderr, "\nin server, the project %s already exists\n\n", projectname);
		return 0; 
	}
	else {
		fprintf(stderr, "\nin server, mkdir() error. path=|%s|\n", path);
		return 1;
	}
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    fprintf(stderr, "in server, creating projectname = |%s|...\n", projectname);
    strcat(path,"/master");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    strcat(path,"/.Manifest");
    fprintf(stderr, "creating path=|%s|\n", path);
    int fileDesc=open(path, O_WRONLY|O_TRUNC|O_CREAT, 0744);
    write(fileDesc, "0\n", 2);
    close(fileDesc);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);

    fprintf(stderr, "in server, sent execution status to client. status=|%s|\n\n", status);

    // send project master version .manifest to client 
    // ** send file:
    //    1) read byte stream from "path" on (server), and 
    //    2) write to "connect_socket" (client) 
    if (send1file(connect_socket, path) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n", path);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n\n", path);

    free(path);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
};

int process_destroy(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'destroy' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);
    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    char * filePath = (char*)malloc((50*sizeof(char)) + sizeof(projectname));

    strcpy(filePath,"Repository/");
    mkdir(filePath, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(filePath,projectname);
    if (!isDir(filePath)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // ----------------------------------------------------------------
    // remove project folder......
    // ----------------------------------------------------------------
    char projectPath[BUFSIZ];
    char commandLine[BUFSIZ];
    strcpy(projectPath,"Repository/");
    strcat(projectPath,projectname);
    strcpy(commandLine, "rm -rf ");
    strcat(commandLine, projectPath);
//    fprintf(stderr, "\ncommandLine = |%s|\n", commandLine);    
    system(commandLine);

//    removingDirectory(projectPath);

    // ----------------------------------------------------------------
    // send command execution status back to client
    // ----------------------------------------------------------------
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent command execution status to client. status=|%s|\n", status);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    // ----------------------------------------------------------------
    // ** (release) mutex
    //    project destroyed
    // ----------------------------------------------------------------
    if (lock_index >= 0) {
	strcpy(pLock_name[lock_index],"");
   	fprintf(stderr, "** in server, mutex released, projectname=|%s|.\n\n", projectname);
    }

    return 0;
};

int process_currentversion(void *client_ptr)
{
        struct client *ctl = client_ptr;
        int connect_socket = ctl->client_sockfd;
        char projectname[BUFSIZ];
        char status[BUFSIZ];

        fprintf(stderr, "\nin server, received command 'currentversion' from client.\n");

        recv(connect_socket, projectname, sizeof(projectname), 0);
    	fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

        char * filePath = (char*)malloc((50*sizeof(char)) + sizeof(projectname));
        strcpy(filePath,"Repository/");
    	mkdir(filePath, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

        strcat(filePath,projectname);
    	if (!isDir(filePath)) {
 		// send command execution status back to client
 		memset(status, 0, sizeof(status));
 		strcpy(status, "ProjectDoesNotExist");
 		send(connect_socket, status, sizeof(status), 0);
		// print message from server console
		fprintf(stderr, "\nin server, the project %s does not exist\n\n", projectname);
		return 0; 
    	}

        // ----------------------------------------------------------------
        // ** mutex lock 
        //    >> enter "critical section"
        // ----------------------------------------------------------------
        int lock_index = mutex_lock(projectname);

    	// send command execution status back to client
    	memset(status, 0, sizeof(status));
    	strcpy(status, "continue");
    	send(connect_socket, status, sizeof(status), 0);
    	fprintf(stderr, "in server, sent execution status to client. status=|%s|\n", status);

    	// ** send file:
        strcat(filePath,"/master/");
        strcat(filePath,".Manifest");
    	if (send1file(connect_socket, filePath) != 0) {
		fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n", filePath);
		return 1;
    	};
    	fprintf(stderr, "** in server: sent path=|%s|\n", filePath);

	free(filePath);

        // ----------------------------------------------------------------
        // ** (unlock) mutex 
        //    >> exit "critical section" 
        // ----------------------------------------------------------------
        mutex_unlock(projectname, lock_index);

    	return 0;
};

int process_history(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
    char file_path[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'history' from client.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);
    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    char * path=(char *)malloc(50+sizeof(projectname));
    strcpy(path,"Repository/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(path,projectname);
    if (!isDir(path)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent execution status to client. status=|%s|\n", status);

    // ** send file:
    strcat(path,"/.History");
    if (send1file(connect_socket, path) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n\n", path);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n", path);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
};

int process_checkout(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
    char file_path[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'checkout' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);
    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    strcpy(file_path,"Repository/");
    mkdir(file_path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(file_path,projectname);
    if (!isDir(file_path)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");

    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent execution status to client. status=|%s|\n", status);

    // ----------------------------------------------------------------
    // zip project "master" folder to project.tar.bz2
    // when unzip, back to "master" folder
    // ----------------------------------------------------------------
    char bz2Path[BUFSIZ];
    char projPath[BUFSIZ];
    char cwd[BUFSIZ];

    strcpy(projPath,"Repository/");
    strcat(projPath,projectname);

    chdir(projPath);
//    getcwd(cwd, sizeof(cwd));
//    printf("\ncwd (2) = |%s|\n", cwd);

    strcpy(bz2Path,"master.tar.bz2");
//    printf("\nbz2Path = |%s|\n",bz2Path);    
    remove(bz2Path);	// remove bz2Path if pre-exist
   
    char commandLine[BUFSIZ];
    strcpy(commandLine,"tar cvfj master.tar.bz2 master");
    printf("commandLine=|%s|\n",commandLine);
    system(commandLine);

    chdir("..");
    chdir("..");
//    getcwd(cwd, sizeof(cwd));
//    printf("\ncwd (0) = |%s|\n", cwd);

    strcpy(bz2Path, projPath);
    strcat(bz2Path, "/master.tar.bz2");

    // ** send file:
    //    1) read byte stream from "path" on (server), and 
    //    2) write to "connect_socket" (client) 
    if (send1file(connect_socket, bz2Path) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n", bz2Path);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n", bz2Path);

    // clean up
    remove(bz2Path);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent final status to client. status=|%s|\n", status);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
}

int process_update(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
//    char cont[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'update' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);

    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);
    
    char * path=(char *)malloc(50+sizeof(projectname));
    strcpy(path,"Repository/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(path,projectname);
    if (!isDir(path)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n\n", status);

//    fprintf(stderr, "\npath = |%s|\n", path);

    // send project master version .manifest to client 
    // ** send file:
    //    1) read byte stream from "path" on (server), and 
    //    2) write to "connect_socket" (client) 
    strcat(path,"/master/.Manifest");
    if (send1file(connect_socket, path) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n", path);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n\n", path);

    // receive status from client
    recv(connect_socket, status, sizeof(status), 0);
    if (strcmp(status, "succeeded") != 0) {
   	fprintf(stderr, "\nError: received client status=|%s|\n\n", status);
	return 1;
    }
    fprintf(stderr, "in server, received client status=|%s|\n\n", status);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
};

int process_upgrade(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
    char filepath[BUFSIZ];
 
    struct LLnode * LLnodePtr;

    fprintf(stderr, "\nin server, received command 'upgrade' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);

    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    char * path=(char *)malloc(50+sizeof(projectname));
    strcpy(path,"Repository/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(path,projectname);
    if (!isDir(path)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n\n", status);

    // work with client to apply command in .Update
    recv(connect_socket, status, sizeof(status), 0);
    while (strcmp(status, "continue")!=0) {
    //in each of these, make the change and update the manifest linked list
	if (strcmp(status, "M")==0) {
		recv(connect_socket, status, sizeof(status), 0);
		strcpy(filepath, "Repository/");
		strcat(filepath, projectname);
		strcat(filepath, "/master/");
		strcat(filepath, status);
		if (send1file(connect_socket, filepath) != 0) {
		  fprintf(stderr, "\nError: send1file(connect_socket, filepath), filepath=|%s|\n",
				filepath);
		  return 1;
		};
		fprintf(stderr, "** in server: sent filepath=|%s|\n", filepath);
	}
	if (strcmp(status, "A")==0) {		// A=Add
		recv(connect_socket, status, sizeof(status), 0);
		strcpy(filepath, "Repository/");
		strcat(filepath, projectname);
		strcat(filepath, "/master/");
		strcat(filepath, status);
		if (send1file(connect_socket, filepath) != 0) {
		  fprintf(stderr, "\nError: send1file(connect_socket, filepath), filepath=|%s|\n",
				filepath);
		  return 1;
		};
		fprintf(stderr, "** in server: sent filepath=|%s|\n", filepath);
	}
    	recv(connect_socket, status, sizeof(status), 0);
    }
    fprintf(stderr, "in server, received client status=|%s|\n", status);

    // -----------------------------------------------------------------------------
    // client upgraded, send server .Manifest to client 
    // -----------------------------------------------------------------------------
    strcpy(filepath,"Repository/");
    strcat(filepath,projectname);
    strcat(filepath,"/master/.Manifest");
    if (send1file(connect_socket, filepath) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n", filepath);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n\n", filepath);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n", status);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
}

int process_commit(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
//    char cont[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'commit' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);

    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);
    
    char * path=(char *)malloc(50+sizeof(projectname));
    strcpy(path,"Repository/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(path,projectname);
    if (!isDir(path)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n", status);

//    fprintf(stderr, "\npath = |%s|\n", path);

    // send project master version .manifest to client 
    // ** send file:
    //    1) read byte stream from "path" on (server), and 
    //    2) write to "connect_socket" (client) 
    strcat(path,"/master/.Manifest");
    if (send1file(connect_socket, path) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n\n", path);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n", path);

    // receive status from client
    recv(connect_socket, status, sizeof(status), 0);
    if (strcmp(status, "continue") != 0) {
   	fprintf(stderr, "\nError: received client status. status=|%s|\n\n", status);
	return 1;
    }
    fprintf(stderr, "in server, received client status. status=|%s|\n", status);

    // ** receive file:
    //    1) read file byte stream from connect_socket (client) and 
    //    2) save to filePath_toSave on (server)
    char filePath_toSave[BUFSIZ];
    strcpy(filePath_toSave, "Repository/");
    strcat(filePath_toSave, projectname);
    strcat(filePath_toSave, "/.CommitActive");
    if (receive1file(connect_socket, filePath_toSave) != 0) 
    {
	fprintf(stderr, "\nError: receive1file(connect_socket, filePath_toSave), filePath_toSave=|%s|\n\n",
		filePath_toSave);
	return 1;
    };
    fprintf(stderr, "** in server: received filePath_toSave=|%s|\n", filePath_toSave);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent command execution status to server. status=|%s|\n", status);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
};

int process_push(void *client_ptr)
{
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
    char filepath[BUFSIZ];
 
    struct LLnode * LLnodePtr;

    fprintf(stderr, "\nin server, received command 'push' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);

    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    char * path=(char *)malloc(50+sizeof(projectname));
    strcpy(path,"Repository/");
    mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(path,projectname);
    if (!isDir(path)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // check "/.CommitActive"
    strcat(path,"/.CommitActive");
    if( (access( path, F_OK )) == -1 ) //need .CommitActive
    {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, ".CommitActive_DoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the .CommitActive file should be present\n"); 
	return 0;
    }

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n", status);

    // ** receive file:
    //    1) read file byte stream from connect_socket (client) and 
    //    2) save to filePath_toSave on (server)
    char filePath_toSave[BUFSIZ];
    strcpy(filePath_toSave, "Repository/");
    strcat(filePath_toSave, projectname);
    strcat(filePath_toSave, "/.CommitSecond");
    if (receive1file(connect_socket, filePath_toSave) != 0) 
    {
	fprintf(stderr, "\nError: receive1file(connect_socket, filePath_toSave), filePath_toSave=|%s|\n",
		filePath_toSave);
	return 1;
    };
    fprintf(stderr, "** in server: received filePath_toSave=|%s|\n", filePath_toSave);

    // compare .CommitActive and .CommitSecond
    // both should be in (project folder), not (master folder) 
    char commitActivePath[BUFSIZ];
    char commitSecondPath[BUFSIZ];
    strcpy(commitActivePath,"Repository/");
    strcat(commitActivePath,projectname);
    strcat(commitActivePath,"/.CommitActive");
    strcpy(commitSecondPath,"Repository/");
    strcat(commitSecondPath,projectname);
    strcat(commitSecondPath,"/.CommitSecond");
    // use SHA1 hash key to compare 2 files
    char * activeHash=getFileSHA1(commitActivePath);
    char * secondHash=getFileSHA1(commitSecondPath);
    if (strcmp(activeHash,secondHash) != 0) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, ".CommitExpired_PleaseCommitAgain");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "in server, .CommitSecond and .CommitActive are not the same.\n"); 
	return 0;
    }

    // --------------------------------------------
    //copy from server manifest to linked list
    // --------------------------------------------
    struct LLnode * serverHead=(struct LLnode *)malloc(sizeof(struct LLnode));
    serverHead->path=NULL;
    serverHead->next = NULL;

    char manifestPath[BUFSIZ];
    strcpy(manifestPath,"Repository/");
    strcat(manifestPath,projectname);
    strcat(manifestPath,"/master/.Manifest");

//printf("\nmanifestPath=|%s|\n",manifestPath);

    int fileDesc=open(manifestPath, O_RDONLY, 0744);
    int fsize=fileSize(manifestPath);
    char* contents=(char*)malloc((fsize+20)*sizeof(char));
    contents[0]='\0';
    if (read(fileDesc, contents, fsize)==0) { //error
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ServerError::read()");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, error: read(fileDesc, contents, fsize)==0\n"); 
	return 1;
    }
    contents[fsize]='\0';

    int i=0;
    int numTokens=0; //projNum,newline,path,tab,fileVersionNum,tab,hash,newline
    char projVersionNumServer[BUFSIZ];
    char * token=nextToken(contents,i,strlen(contents));//projNum
    i=i+strlen(token);
    strcpy(projVersionNumServer,token);
    free(token); // from nextToken() via malloc, free if no more use 

    numTokens=0;
    LLnodePtr=serverHead;
    while (i<strlen(contents)) {
	token=nextToken(contents,i,strlen(contents));

// printf("\ntoken=|%s|\n",token);

	if (numTokens % 6==1) { //token is the path
		if (numTokens!=1) {
			LLnodePtr->next=(struct LLnode *)malloc(sizeof(struct LLnode));
			LLnodePtr=LLnodePtr->next;
			LLnodePtr->next=NULL;
		}
		LLnodePtr->path=token;
		LLnodePtr->next=NULL;
	}
        if (numTokens % 6==3) { //token is the version Number
		LLnodePtr->vNum=token;
	}
        if (numTokens % 6==5) { //token is the hash
		LLnodePtr->hash=token;
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

    LLnodePtr=serverHead;
    while (LLnodePtr != NULL) {
	printf("\nLLnodePtr->path=|%s|,LLnodePtr->vNum=|%s|,LLnodePtr->hash=|%s|\n",
		LLnodePtr->path,LLnodePtr->vNum,LLnodePtr->hash);
	LLnodePtr = LLnodePtr->next;
    }

    // ----------------------------------------------------------------
    // zip "master" folder to vN.tar.bz2, where N is project version
    // when unzip, back to "master" folder
    // ----------------------------------------------------------------
    char bz2Path[BUFSIZ];
    strcpy(bz2Path,"Repository/");
    strcat(bz2Path,projectname);
    strcat(bz2Path,"/v");
    strcat(bz2Path,projVersionNumServer);
    strcat(bz2Path,".tar.bz2");
//    printf("\nbz2Path = |%s|\n",bz2Path);    
    remove(bz2Path);	// remove bz2Path if pre-exist
   
    char commandLine[BUFSIZ];
    strcpy(commandLine,"tar cvfj Repository/");
    strcat(commandLine,projectname);
    strcat(commandLine,"/v");
    strcat(commandLine,projVersionNumServer);
    strcat(commandLine,".tar.bz2 Repository/");
    strcat(commandLine,projectname);
    strcat(commandLine,"/master");
    printf("\ncommandLine=|%s|\n",commandLine);
    system(commandLine);

    // send command execution status back to client
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n", status);

    // work with client to apply command in .Commit
    recv(connect_socket, status, sizeof(status), 0);
    while (strcmp(status, "succeeded")!=0) {
    //in each of these, make the change and update the manifest linked list
	if (strcmp(status, "Copy")==0) {
		recv(connect_socket, status, sizeof(status), 0);
		strcpy(filePath_toSave, "Repository/");
		strcat(filePath_toSave, projectname);
		strcat(filePath_toSave, "/master/");
		strcat(filePath_toSave, status);
		if (receive1file(connect_socket, filePath_toSave) != 0) // override if exist
		{
		  fprintf(stderr, 
		    "\nError: receive1file(connect_socket, filePath_toSave), filePath_toSave=|%s|\n",
		    filePath_toSave);
			return 1;
		};
		fprintf(stderr, "\n** in server: received filePath_toSave=|%s|\n", filePath_toSave);
		// update linked list (.Manifest)
		struct LLnode * ptr=LLsearch(serverHead,status);
		ptr->hash=getFileSHA1(filePath_toSave);
		int num = atoi(ptr->vNum);
    		num=num+1;
    		ptr->vNum[0]='\0';
		itoa(num, ptr->vNum);
	}
	if (strcmp(status, "Add")==0) {		// Add=Insert
		recv(connect_socket, status, sizeof(status), 0);
		strcpy(filePath_toSave, "Repository/");
		strcat(filePath_toSave, projectname);
		strcat(filePath_toSave, "/master/");
		strcat(filePath_toSave, status);
		if (receive1file(connect_socket, filePath_toSave) != 0) // override if exist
		{
		  fprintf(stderr, 
		    "\nError: receive1file(connect_socket, filePath_toSave), filePath_toSave=|%s|\n",
		    filePath_toSave);
			return 1;
		};
		fprintf(stderr, "\n** in server: received filePath_toSave=|%s|\n", filePath_toSave);
		// update linked list (.Manifest)
		char * hash=getFileSHA1(filePath_toSave);
	    	char * tmpPath=(char*)malloc((100)*sizeof(char));
		strcpy(tmpPath, status); 
		serverHead=LLinsert(serverHead, tmpPath, "1", hash);
	}
	if (strcmp(status, "Remove")==0) {	// Remove=Delete
		recv(connect_socket, status, sizeof(status), 0);
		strcpy(filePath_toSave, "Repository/");
		strcat(filePath_toSave, projectname);
		strcat(filePath_toSave, "/master/");
		strcat(filePath_toSave, status);
		int ret = remove(filePath_toSave);
		if (ret == 0) {
			fprintf(stderr, "\n** in server: removed file=|%s|\n", filePath_toSave);
		}
		else {
			fprintf(stderr, "\n** warning: cannot remove file=|%s|\n", filePath_toSave);
		}			
		serverHead=LLdelete(serverHead,status);
	}
    	recv(connect_socket, status, sizeof(status), 0);
    }

    fprintf(stderr, "in server, received client status. status=|%s|\n", status);

    //copy the manifest linked list to the manifest file
    char * s=(char *)malloc(20);
    s[0]='\0';
    //increment version number
    int num = atoi(projVersionNumServer);
    num=num+1;
    projVersionNumServer[0]='\0';
    itoa(num, projVersionNumServer);
    strcpy(s,projVersionNumServer);
    strcat(s,"\n");
    struct LLnode * node_ptr=serverHead;
    while (node_ptr!=NULL) {
	int size=strlen(node_ptr->path)+strlen(node_ptr->hash)+strlen(node_ptr->vNum)+10;
	s=readjustBig(s,size); 
	strcat(s,node_ptr->path);
	strcat(s,"\t");
	strcat(s,node_ptr->vNum);
	strcat(s,"\t");
	strcat(s,node_ptr->hash);
	strcat(s,"\n");
	node_ptr=node_ptr->next;
    }
    // write to file
    strcpy(manifestPath,"Repository/");
    strcat(manifestPath,projectname);
    strcat(manifestPath,"/master/.Manifest");
    fileDesc=open(manifestPath, O_WRONLY|O_TRUNC|O_CREAT, 0744);
    write(fileDesc, s, strlen(s));
    close(fileDesc);

    // ----------------------------------------------------------------
    // send command execution status back to client
    // ----------------------------------------------------------------
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent command execution status to client. status=|%s|\n", status);

    // -----------------------------------------------------------------------------
    // ** "Push" succeeded, .Commit applied, server project version advanced
    //    now send the updated server .Manifest, to override the client .Manifest 
    // -----------------------------------------------------------------------------
    strcpy(manifestPath,"Repository/");
    strcat(manifestPath,projectname);
    strcat(manifestPath,"/master/.Manifest");
    if (send1file(connect_socket, manifestPath) != 0) {
	fprintf(stderr, "\nError: send1file(connect_socket, path), path=|%s|\n", manifestPath);
	return 1;
    };
    fprintf(stderr, "** in server: sent path=|%s|\n", manifestPath);

    // --------------------------------------------
    // write to .History
    // --------------------------------------------
    char logPath[BUFSIZ];
    strcpy(logPath,"Repository/");
    strcat(logPath,projectname);
    strcat(logPath,"/.History");

    // return 1:file; return 0:directory; return -1:invalid name
    if (isFile(logPath) == 0) {		// directory
      	fprintf(stderr, "\nError: |%s| is a folder, not a file.\n\n", logPath);
	return 1;
    }
    else if (isFile(logPath) == -1) {	// file not exist
	if (create_file(logPath) != 0) {
	      	fprintf(stderr, "\nError: create file |%s|.\n\n", logPath);
		return 1;
	}

    }

    int fd_read;
    int fd_write;
    int fsizeCommit;
    ssize_t ret_in;    /* byte-count returned by read() */
    int ret_out; 

    fd_write = open(logPath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd_write < 0) {
	fprintf(stderr, "\nError: open in_file (%s)\n", logPath);
	return 1;
    }

    ret_out = write(fd_write, "\npush\n", strlen("\npush\n"));
    if (ret_out == -1) {
	fprintf(stderr, "\nError: write(), file=|%s|, message=|%s|\n", strerror(errno), logPath);
    }
    write(fd_write, projVersionNumServer, strlen(projVersionNumServer));
    write(fd_write, "\n", 1);

    fd_read=open(commitActivePath,O_RDONLY);
    if(fd_read < 0)
    {
	fprintf(stderr, "\nError: open in_file (%s)\n", commitActivePath);
	return 1;
    }

    if ((fsizeCommit=fileSize(commitActivePath)) == -1) {
    	fprintf(stderr, "\nError: fileSize() from file: %s\n", commitActivePath);
      	return 1;
    }

    char * contentsCommit=(char *)malloc(fsizeCommit*sizeof(char));
    if (contentsCommit==NULL) {
    	fprintf(stderr, "\nError: malloc() for contentsCommit \n");
      	return 1;
    }
    if ((ret_in = read(fd_read, contentsCommit, fsizeCommit)) == -1) { // error
	fprintf(stderr, "\nError: read() from file: %s\n", commitActivePath);
      	return 1;
    }
    if (ret_in != fsizeCommit) { // did not read all bytes from file
	fprintf(stderr, "\nDid not read all bytes from file:\n");
    	fprintf(stderr, "\n  file: %s; ret_in: %ld; fsizeCommit: %d\n", commitActivePath, ret_in, fsizeCommit);
      	return 1;
    }
    if (ret_in == 0) { // nothing to read
    	fprintf(stderr, "\nNothing to read from file: %s\n", commitActivePath);
      	return 1;
    }

    write(fd_write, contentsCommit, fsizeCommit);

    close(fd_read);
    close(fd_write);
    free(contentsCommit);

    // -----------------------------------------------------
    // archive project/.CommitActive to master/.Commit
    // -----------------------------------------------------
    char cmd[BUFSIZ];

    strcpy(cmd, "cp ");
    strcat(cmd, commitActivePath);
    strcat(cmd, " Repository/");
    strcat(cmd, projectname);
    strcat(cmd, "/master/.Commit");

    system(cmd);

    // send command execution status back to client

    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent command execution status to client. status=|%s|\n", status);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
};

int process_rollback(void *client_ptr)
{       
    struct client *ctl = client_ptr;
    int connect_socket = ctl->client_sockfd;
    char projectname[BUFSIZ];
    char status[BUFSIZ];
    char rollbackVers[BUFSIZ];

    fprintf(stderr, "\nin server, received command 'rollback' from client.\n");
//    fprintf(stderr, "\nin server, need to get 'projectname' next.\n");

    recv(connect_socket, projectname, sizeof(projectname), 0);
    fprintf(stderr, "in server, received projectname = |%s|.\n", projectname);

    char * filePath = (char*)malloc((50*sizeof(char)) + sizeof(projectname));

    strcpy(filePath,"Repository/");
    mkdir(filePath, S_IRWXU | S_IRWXG | S_IRWXO);  // create Repository folder if not exist

    strcat(filePath,projectname);
    if (!isDir(filePath)) {
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, "ProjectDoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, the project %s does not exist\n", projectname);
	return 0; 
    }

    // ----------------------------------------------------------------
    // ** mutex lock 
    //    >> enter "critical section"
    // ----------------------------------------------------------------
    int lock_index = mutex_lock(projectname);

    // -------------------------------------------------------------------
    // send command execution status back to client
    // -------------------------------------------------------------------
    memset(status, 0, sizeof(status));
    strcpy(status, "continue");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent status to client. status=|%s|\n", status);

    recv(connect_socket, rollbackVers, sizeof(rollbackVers), 0);
    fprintf(stderr, "in server, received rollbackVers = |%s|.\n", rollbackVers);

    // -------------------------------------------------------------------
    // check if version number is invalid
    // -------------------------------------------------------------------
    strcat(filePath,"/master/.Manifest");

    const char delim = '\n';
    char * fileData = readFileDatafromFileName(filePath);
    char * currentVers = strtok(fileData, &delim);
    int rollbackNum = atoi(rollbackVers);
    int currentNum = atoi(currentVers);

//fprintf(stderr, "\nin server, rollbackVers=|%s|, currentVers=|%s|.\n", rollbackVers, currentVers);
    
    if (rollbackNum >= currentNum){
 	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, ".RollbackVersionNotLowerThanCurrentVersion");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, RollbackVersion >= CurrentVersion\n"); 
	return 0;
    }
    free(fileData);

    // -------------------------------------------------------------------
    // check if rollback version .tar.bz2 exists
    // -------------------------------------------------------------------
    char projectPath[BUFSIZ];
    char bz2Path[BUFSIZ];

    strcpy(projectPath, "Repository/");
    strcat(projectPath, projectname);
    strcpy(bz2Path, projectPath);
    strcat(bz2Path,"/v");
    strcat(bz2Path,rollbackVers);
    strcat(bz2Path,".tar.bz2");
    printf("\nbz2Path = |%s|\n",bz2Path);

    if (isFile(bz2Path) != 1) {
	// send command execution status back to client
 	memset(status, 0, sizeof(status));
 	strcpy(status, bz2Path);
	strcat(status, ".DoesNotExist");
 	send(connect_socket, status, sizeof(status), 0);
	// print message from server console
	fprintf(stderr, "\nin server, |%s| does not exist.\n", bz2Path); 
	return 0;
    }

    // -------------------------------------------------------------------
    // remove master folder
    // -------------------------------------------------------------------
    char masterPath[BUFSIZ];
    char commandLine[BUFSIZ];
    strcpy(masterPath, "Repository/");
    strcat(masterPath, projectname);
    strcat(masterPath, "/master");
    strcpy(commandLine, "rm -rf ");
    strcat(commandLine, masterPath);
//    fprintf(stderr, "\ncommandLine = |%s|\n", commandLine);    
    system(commandLine);

    // -------------------------------------------------------------------
    // unzip .tar.bz2 to restore rollback version project
    // -------------------------------------------------------------------
//    strcpy(commandLine, "tar -xvf ");
    strcpy(commandLine, "tar -xf ");
    strcat(commandLine, bz2Path);
    fprintf(stderr, "\ncommandLine = |%s|\n", commandLine);    
    system(commandLine);

    // -------------------------------------------------------------------
    // remove all .tar.bz2 files later then rollback version
    // -------------------------------------------------------------------
    char tmp_bz2Path[BUFSIZ];
    char version_num[BUFSIZ];
    int i;
    for (i = (rollbackNum); i < currentNum; i++) {
	    strcpy(tmp_bz2Path,"Repository/");
	    strcat(tmp_bz2Path,projectname);
	    strcat(tmp_bz2Path,"/v");
	    itoa(i, version_num);
	    strcat(tmp_bz2Path,version_num);
	    strcat(tmp_bz2Path,".tar.bz2");
    	    fprintf(stderr, "\n(remove) tmp_bz2Path = |%s|\n", tmp_bz2Path);    
	    remove(tmp_bz2Path);
    }

    // -------------------------------------------------------------------
    // sent command execution status to client
    // -------------------------------------------------------------------
    memset(status, 0, sizeof(status));
    strcpy(status, "succeeded");
    send(connect_socket, status, sizeof(status), 0);
    fprintf(stderr, "in server, sent command execution status to client. status=|%s|\n\n", status);

    // -------------------------------------------------------------------
    // write to .History
    // -------------------------------------------------------------------
    char logPath[BUFSIZ];
    strcpy(logPath,"Repository/");
    strcat(logPath,projectname);
    strcat(logPath,"/.History");

    // return 1:file; return 0:directory; return -1:invalid name
    if (isFile(logPath) == 0) {		// directory
      	fprintf(stderr, "\nError: |%s| is a folder, not a file.\n\n", logPath);
	return 1;
    }
    else if (isFile(logPath) == -1) {	// file not exist
	if (create_file(logPath) != 0) {
	      	fprintf(stderr, "\nError: create file |%s|.\n\n", logPath);
		return 1;
	}

    }

    int fd_read;
    int fd_write;
    int fsizeCommit;
    ssize_t ret_in;    /* byte-count returned by read() */
    int ret_out; 

    fd_write = open(logPath, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd_write < 0) {
	fprintf(stderr, "\nError: open in_file (%s)\n", logPath);
	return 1;
    }

    ret_out = write(fd_write, "\nrollback\n", strlen("\nrollback\n"));
    if (ret_out == -1) {
	fprintf(stderr, "\nError: write(), file=|%s|, message=|%s|\n", strerror(errno), logPath);
    }
    write(fd_write, rollbackVers, strlen(rollbackVers));
    write(fd_write, "\n", 1);

    char commitPath[BUFSIZ];
    strcpy(commitPath, "Repository/");
    strcat(commitPath, projectname);
    strcat(commitPath, "/master/.Commit");

    fd_read=open(commitPath,O_RDONLY);
    if(fd_read < 0)
    {
	fprintf(stderr, "\nError: open in_file (%s)\n", commitPath);
	return 1;
    }

    if ((fsizeCommit=fileSize(commitPath)) == -1) {
    	fprintf(stderr, "\nError: fileSize() from file: %s\n", commitPath);
      	return 1;
    }


    char * contentsCommit=(char *)malloc(fsizeCommit*sizeof(char));
    if (contentsCommit==NULL) {
    	fprintf(stderr, "\nError: malloc() for contentsCommit \n");
      	return 1;
    }
    if ((ret_in = read(fd_read, contentsCommit, fsizeCommit)) == -1) { // error
	fprintf(stderr, "\nError: read() from file: %s\n", commitPath);
      	return 1;
    }
    if (ret_in != fsizeCommit) { // did not read all bytes from file
	fprintf(stderr, "\nDid not read all bytes from file:\n");
    	fprintf(stderr, "\n  file: %s; ret_in: %ld; fsizeCommit: %d\n", commitPath, ret_in, fsizeCommit);
      	return 1;
    }
    if (ret_in == 0) { // nothing to read
    	fprintf(stderr, "\nNothing to read from file: %s\n", commitPath);
      	return 1;
    }


    write(fd_write, contentsCommit, fsizeCommit);

    close(fd_read);
    close(fd_write);
    free(contentsCommit);

    // ----------------------------------------------------------------
    // ** (unlock) mutex 
    //    >> exit "critical section" 
    // ----------------------------------------------------------------
    mutex_unlock(projectname, lock_index);

    return 0;
};

int init_mutexlock()
{
    int i;
    for (i=0; i<MAX_MUTEXLOCK; i++) {
	strcpy(pLock_name[i], "");
     	if (pthread_mutex_init(&pLock_mutex[i], NULL) != 0) 
    	{ 
		perror("\n** Error: mutex init has failed\n");
		return 1;
    	} 
    }
    return 0;
}

int mutex_lock(char * projectname)
{
    int lock_index = -1;
    int i;
    for (i=0; i<MAX_MUTEXLOCK; i++) {
	if (strcmp(pLock_name[i], projectname) == 0) {
	    fprintf(stderr, "** in server, existing mutex located, projectname=|%s|.\n\n",
		projectname);	
	    break;
	}
    }
    if (i == MAX_MUTEXLOCK) {
	for (i=0; i<MAX_MUTEXLOCK; i++) {
	    if (strcmp(pLock_name[i], "") == 0) {
		strcpy(pLock_name[i], projectname);
		fprintf(stderr, "** in server, new mutex assigned, projectname=|%s|.\n\n",
			projectname);	
		break;
	    }
	}
    }    
    if (i < MAX_MUTEXLOCK) {
	lock_index = i;
    }
    else {
    	fprintf(stderr, "\n** in server, mutex count exceeded, projectname=|%s|.\n\n", projectname);
    }

    // (lock) if mutex available
    if (lock_index >= 0) {
	pthread_mutex_lock(&pLock_mutex[lock_index]); 
    	fprintf(stderr, "** in server, mutex locked, projectname=|%s|.\n\n", projectname);
    }

    return lock_index;
}

int mutex_unlock(char * projectname, int lock_index) {
    if (lock_index >= 0) {
   	pthread_mutex_unlock(&pLock_mutex[lock_index]); 
   	fprintf(stderr, "** in server, mutex unlocked, projectname=|%s|.\n\n", projectname);
    }
}



