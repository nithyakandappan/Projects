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


struct Node {
	int frequency;
	char* token;
	struct Node* left;
	struct Node* right;
	int height;
};

int heapIndex=1;
struct Node ** heap;

char * readjustBig(char * s, int i);	//
int nextNumber();
void convertAVLtoHeap(struct Node* root, int i);
struct Node* insert(char* input,struct Node* start);
int right(int i); //
int left(int i); //
int parent(int i); //
void insertToHeap(struct Node * value, int size); //
void increaseValueAtIndex(int index, struct Node* newValue); //
struct Node returnMin(int size); //
void deleteMin(int size); //
void delete(int i, int size); //
void buildMinHeap(int size); //
void minHeapify(int i,int size); //
void allocateHeap(int size); //
char * readjust(char * s);
off_t fileSize(const char *filename); //
void readingFiles(char * folder);
int isDirBC(const char* target); //
char * nextToken(char * contents, int i,int fsize);
void getTokens(char * fileName);
struct Node* buildHuffmanTree(int size, struct Node * root);
char * generateAllCodes(int size, char * str, struct Node * hroot, char * s);
char * generateCode(struct Node* root, int size);
struct Node * makeTreeFromDirectory(char * folder,struct Node * root); //
struct Node * makeTreeFromFile(char * fileName,struct Node * root); //
int sizeOfTree(struct Node* root);
void printTree(struct Node * root);
char * convertWhiteSpace(char * str);
struct Node* rotateRight(struct Node* rotation);
struct Node* rotateLeft(struct Node* rotation);
int max(struct Node* n1);
int isFile(const char* name);


void allocateHeap(int size) {
	struct Node** heap=(struct Node **)malloc(size*sizeof(struct Node *));
}

void minHeapify(int i,int size) {
	
	int smallest;
	if (i<1) {
		return;
	}
	if (left(i)<=size && heap[left(i)]->frequency<=heap[i]->frequency) {
		smallest=left(i);
	}
	else {
		smallest=i;
	}
	if (right(i)<=size && heap[right(i)]->frequency<=heap[smallest]->frequency) {
		smallest=right(i);
	}
	if (smallest != i) {
		struct Node temp = *(heap[i]) ;
    		*(heap[i]) = *(heap[smallest]) ;
    		*(heap[smallest]) = temp ;
		minHeapify(smallest,size);
		
	}
}

void buildMinHeap(int size) { //heap starts with array 1
	int i;
	for (i=size/2;i>=1;i--) {
		minHeapify(i,size);
	}
}
/*
void printHeap(struct Node** array, int size) {
	int i;
	for (i=1;i<=size;i++) {
		printf("frequency: %d\n",array[i]->frequency);
	}
	printf("\n\n\n");
}
*/

void delete(int i, int size) { //deletes ket at index a[i]
	struct Node temp=*heap[i];
	*heap[i]=*heap[size];
	*heap[size]=temp;
	minHeapify(i,size-1);
}

void deleteMin(int size) {
	delete(1,size);
}

struct Node returnMin(int size) {
	return *heap[1];
}

void increaseValueAtIndex(int index, struct Node* value) {
	heap[index]=value;
	while (index>1 && heap[parent(index)]->frequency>heap[index]->frequency) {
		struct Node temp=*heap[index];
		*heap[index]=*heap[parent(index)];
		*heap[parent(index)]=temp;
		index=parent(index);
	}
}

void insertToHeap(struct Node * value, int size) {
	heap[size+1]->frequency=-1; //frequencies must be 1 or more
	increaseValueAtIndex(size+1,value);
}

int parent(int i) {
	return i/2;
}

int left(int i) {
	return 2*i;
}

int right(int i) {
	return 2*i+1;
}

struct Node* insert(char* input, struct Node* start) {
	if (start==NULL){
		struct Node* newnode = (struct Node*) malloc(sizeof(struct Node));
		newnode->token = input;
		newnode->left = NULL;
		newnode->right = NULL;
		newnode->height = 1;
		newnode->frequency = 1;
		return(newnode);
	} else{
		int comp = strcmp(input,start->token);
		//comp is negative -> input is less than start.value
		//comp is positive -> input is greater than start.value
		//comp is zer0 -> they're the same value
		if(comp==0){
			start->frequency+=1;
			return start;
		} else if(comp<0){
			start->left = insert(input,start->left);
		} else{
			start->right = insert(input,start->right);
		}
		//updating height of current node 
		start->height = max(start)+1;

		//checking balance
		int balance;
		int leftHeight = 0;
		int rightHeight = 0;

		int balanceCompR = 0;
		int balanceCompL = 0;

		if (start->left){
			leftHeight = start->left->height;
			balanceCompL = strcmp(input,start->left->token);
			//negative -> input is less than start.left.value
			//positive -> input is greater than start.left.value
		}

		if (start->right){
			rightHeight = start->right->height;
			balanceCompR = strcmp(input,start->right->token);
			//negative -> input is less than start.right.value
			//positive -> input is greater than start.right.value
		}
    
		balance = leftHeight - rightHeight;
		//Right Right case
		if ((balance < -1) && (balanceCompR>0)){
			return rotateLeft(start);
		//Right Left case
		} else if((balance < -1) && (balanceCompR<0)){
			start->right = rotateRight(start->right);
			return rotateLeft(start);
		//Left Left case
		} else if ((balance > 1)&&(balanceCompL<0)){
			return rotateRight(start);
		//Left Right case
		} else if((balance>1)&&(balanceCompL>0)) {
			start->left = rotateLeft(start->left);
			return rotateRight(start);
		}
		return start;
	}
}

void convertAVLtoHeap(struct Node* root, int i) { //first time called, i will be one and heap is empty BUT THE HEAP WILL BE INITIALIZED TO HAVE SPACE FOR 2*SIZE+1 ELEMENTS
	if (root!=NULL) {
		if (root->left!=NULL) {
			convertAVLtoHeap(root->left,nextNumber());
		}
		if (root->right!=NULL) {
			convertAVLtoHeap(root->right,nextNumber());
		}
		heap[i]=(struct Node *)malloc(1*sizeof(struct Node));
		*heap[i]=*root;
	}
}

int nextNumber() {
	heapIndex=heapIndex+1;
	return heapIndex;
}

struct Node* buildHuffmanTree(int size, struct Node * root) { //there might be a problem with delete and insert, and the pointers. 
	struct Node * subTree1=(struct Node *)malloc(sizeof(struct Node));
	subTree1->left=heap[1]->left;
	subTree1->right=heap[1]->right;
	subTree1->frequency=heap[1]->frequency;
	subTree1->token=heap[1]->token;
	deleteMin(size);
	size=size-1;
	if (size==0) {
		return subTree1;
	}
	struct Node * subTree2=(struct Node *)malloc(sizeof(struct Node));
	subTree2->left=heap[1]->left;
	subTree2->right=heap[1]->right;
	subTree2->frequency=heap[1]->frequency;
	subTree2->token=heap[1]->token;
	deleteMin(size);
	size=size-1;
	struct Node * newTree;
	newTree=(struct Node *)malloc(1*sizeof(struct Node));
	newTree->frequency=(subTree1->frequency)+(subTree2->frequency);
	newTree->left=subTree1;
	newTree->right=subTree2;
	if (subTree1->token!=NULL) {
		subTree1->left=NULL;
		subTree1->right=NULL;
	}
	if (subTree2->token!=NULL) {
		subTree2->left=NULL;
		subTree2->right=NULL;
	}	
	insertToHeap(newTree,size);
	size=size+1;
	return buildHuffmanTree(size,root);
}



char * generateAllCodes(int height, char * str,struct Node * hroot, char * code) { //root is the bst , sizeAllocated is the size allocated to the string so far, height is the height of the tree
//returns string that will be outputted to the file so far
	if (hroot!=NULL) {
		if (hroot->left==NULL  && hroot->right==NULL) { //leaf
			str=readjustBig(str,strlen(code)+strlen(hroot->token)+5);//all space needed for the new line
			char * copyT=(char *)malloc(sizeof(char)*2);
			copyT[0]='\t';
			copyT[1]='\0';
			strcat(str,code);
			strcat(str,copyT);
			free(copyT);
			char * temp=(char *)malloc((sizeof(char)*strlen(hroot->token))+1);
			strcpy(temp,hroot->token);
			if (strlen(temp)==1) {
				if (isspace(temp[0])) {
					temp=convertWhiteSpace(temp);
				}
			}
			strcat(str,temp);
			char * copyN=(char *)malloc(sizeof(char)*2);
			copyN[0]='\n';
			copyN[1]='\0';
			strcat(str,copyN);
		}
		if (hroot->left!=NULL) {
			char * newCode=(char *)malloc(height*sizeof(char));
			strcpy(newCode,code);
			char * symbol=(char *)malloc(sizeof(char)*2);
			symbol[0]='0';
			symbol[1]='\0';
			strcat(newCode,symbol);
			str=generateAllCodes(height,str,hroot->left,newCode);
			free(newCode);
		}
		if (hroot->right!=NULL) {
			char * newCode=(char *)malloc(height*sizeof(char));
			strcpy(newCode,code);
			char * symbol=(char *)malloc(sizeof(char)*2);
			symbol[0]='1';
			symbol[1]='\0';
			strcat(newCode,symbol);
			str=generateAllCodes(height,str,hroot->right,newCode);
			free(newCode);
		}
	}
	return str;
}

char * convertWhiteSpace(char * str) {
	char * newStr=(char *)malloc(5*sizeof(char));
	switch ((char unsigned) str[0]) {
			case WHITESPACECHARS_SPC_ASCIICODE:	
				strcpy(newStr,WHITESPACECHARS_SPC_TOKEN);
				break;
			case WHITESPACECHARS_TAB_ASCIICODE:	
				strcpy(newStr,WHITESPACECHARS_TAB_TOKEN);
				break;
			case WHITESPACECHARS_LF_ASCIICODE:
				strcpy(newStr,WHITESPACECHARS_LF_TOKEN);
				break;
			case WHITESPACECHARS_VT_ASCIICODE:	
				strcpy(newStr,WHITESPACECHARS_VT_TOKEN);
				break;
			case WHITESPACECHARS_FF_ASCIICODE:	
				strcpy(newStr,WHITESPACECHARS_FF_TOKEN);
				break;
			case WHITESPACECHARS_CR_ASCIICODE:	
				strcpy(newStr,WHITESPACECHARS_CR_TOKEN);
				break;
			default:
				printf("\nUnrecognized WHITESPACE code: (%d)\n", (char unsigned) str[0]);
	}
	free(str);
	return newStr;
}


void getTokens(char * fileName) {
	int fdesc=open(fileName,O_RDONLY);
	int fsize=fileSize(fileName);
	char* contents=(char*)malloc((fsize+1)*sizeof(char));
	if (read(fdesc, contents, fsize)==0) { //error
		return;
	}
	int i;
	while(i<fsize) {
		char * next=nextToken(contents,i,fsize);
		printf("%s\n",next);
		i=i+strlen(next);
	}
}

char * nextToken(char * contents, int i,int fsize) {
	char * token=(char*)malloc(11*sizeof(char));
	/*if (isspace(contents[i])) {
		token[0]=contents[i];
		return token;
	}
	*/
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
	}
	return token;
	
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

int isDirBC(const char* target)
{
   struct stat statbuf;
   stat(target, &statbuf);
   return S_ISDIR(statbuf.st_mode);
}

struct Node * makeTreeFromDirectory(char * folder,struct Node * root) {
	DIR * dirp = opendir(folder);
	char * path=(char *)malloc(sizeof(char)*(strlen(folder)+2));
	int len;

	strcpy(path,folder);
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
			strcat(path,copy); //can you do /./abc...?
			strcat(path,dp->d_name); //is it better to make a copy of this string instead of using it directly?
			if (isDirBC(path)) {
				root=makeTreeFromDirectory(path,root);    
			}
			else {
				// skip .hcz file 
				len = strlen(path);
				if ((len >= 5) && (strcmp(&(path[len-4]),".hcz")==0)) {
					printf("\n** .hcz file detected, skipping = %s\n", path);
				}
				else {
					root=makeTreeFromFile(path,root);
				}
			}
	    	} else {
			if (errno == 0) {
				return root;
		  	  	closedir(dirp);
				return NULL;
	       	 	}
			return root;
	     	   	closedir(dirp);
	  	}
	}
	return root;
}

struct Node * makeTreeFromFile(char * fileName,struct Node * root) { 
	int fdesc;
	ssize_t ret_in;    /* byte-count returned by read() */
	int fsize;

	fdesc=open(fileName,O_RDONLY);
	if(fdesc < 0)
	{
	      printf("\nError: open in_file... ");
	      return NULL;
	}
	if ((fsize=fileSize(fileName)) == -1) {
    		printf("\nError: fileSize() from file: %s\n", fileName);
		return NULL;
	};
	char* contents=(char*)malloc((fsize+2)*sizeof(char));
	if (contents==NULL) {
    		printf("\nError: malloc() for contents \n");
		return NULL;
	}
	if ((ret_in = read(fdesc, contents, fsize)) == -1) { // error
    		printf("\nError: read() from file: %s\n", fileName);
		return NULL;
	}
	if (ret_in != fsize) { // did not read all bytes from file
		printf("\nDid not read all bytes from file:\n");
    		printf("\n  file: %s; ret_in: %ld; fsize: %d\n", fileName, ret_in, fsize);
		return NULL;
	}
	if (ret_in == 0) { // nothing to read
    		printf("\nNothing to read from file: %s\n", fileName);
		return NULL;
	}

	int i;
	while(i<fsize) {
		char * next=nextToken(contents,i,fsize);
		root=insert(next,root);
		i=i+strlen(next);
	}
	return root;
}

int sizeOfTree(struct Node* root) { 
	int size=0;
	if (root!=NULL) {
		if (root->left!=NULL) {
			size=size+sizeOfTree(root->left);
		}
		if (root->right!=NULL) {
			size=size+sizeOfTree(root->right);
		}
	//	printf(":%s ",root->token);
		return size+1;
	}
	return size;
}

void printTree(struct Node * root) { 
	int size=0;
	if (root!=NULL) {
		if (root->left!=NULL) {
			printTree(root->left);
		}
		printf("frequency:%d, token:%s\n",root->frequency,root->token);
		if (root->right!=NULL) {
			printTree(root->right);
		}
	//	printf(":%s ",root->token);
	}
}

void buildCodebook(char * path,char * codebookName) {
	int len;
	struct Node * root;

	if (isDirBC(path)) {
		root=makeTreeFromDirectory(path,NULL);
	}
	else {
		// skip .hcz file 
		len = strlen(path);
		if ((len >= 5) && (strcmp(&(path[len-4]),".hcz")==0)) {
			printf("\n** .hcz file detected, skipping = %s\n", path);
			return;
		}
		root=makeTreeFromFile(path,NULL);
	}
//	printTree(root);
	int treeSize=sizeOfTree(root);
	heap=(struct Node **)malloc((sizeof(struct Node *))*((2*treeSize)+1));
	convertAVLtoHeap(root, 1);
	buildMinHeap(treeSize);
	treeSize=sizeOfTree(root);		
	struct Node* huffTree=buildHuffmanTree(treeSize, root);
	char * str=(char *)malloc(sizeof(char)*100);
	char * code=(char *) malloc(sizeof(char)*5);
	code[0]='\0';
	str[0]='\0';
	if (sizeOfTree(huffTree)==1) {
		code[0]='0';
		code[1]='\0';
	}
	str=generateAllCodes(2*treeSize, str,huffTree,code);
	char * outputstr=(char *)malloc(strlen(str)*sizeof(char)+5);
	outputstr[0]='\\';
	outputstr[1]='\n';
	outputstr[2]='\0';
	strcat(outputstr,str);
	int fileDesc=open(codebookName, O_WRONLY|O_TRUNC|O_CREAT, 0744);
  	if(fileDesc < 0)
  	{
      		printf("\nError: open file to write... |%s|", codebookName);
      		return;
  	}
	ssize_t isSuccess=write(fileDesc, outputstr, strlen(outputstr));
	if (isSuccess == -1) {
		printf("\n[write] failed...");
	}
}

int max(struct Node* n1){
	int i = 0;
	int j = 0;
	if (n1->left){
		j=n1->left->height;
	}
	if (n1->right){
		i = n1->right->height;
	}
	if (i<j){
		return i;
	}else{
		return j;
	}
}

struct Node* rotateRight(struct Node* rotation){
	struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
	
	//rotation
	temp = rotation->left;
	rotation->left = temp-> right;
	temp->right = rotation;
	
	//updating heights
	rotation->height = max(rotation)+1;
	temp->height = max(temp)+1;
	
	return temp;
}
struct Node* rotateLeft(struct Node* rotation){
	struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
	
	//rotation
	temp = rotation->right;
	rotation->right = temp->left;
	temp->left = rotation;
	
	//updating heights
	rotation->height = max(rotation)+1;
	temp->height = max(temp)+1;
	
	return temp;
}

	

/**int main(int argc, char** argv) {
	//buildCodebook("sample2.txt","codebookTest2.txt");
	buildCodebook("test2.txt","codebookTest2_2.txt");
	//buildCodebook("testfolder/","codebookTestfolder.txt");
	//buildCodebook("testfolder/oneword.txt.hcz","codebookTestfolder.txt");	
}**/
