#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//static int debug = 2;

/* Linked list structure */
struct Node{
  char data[100]; /* assuming that no token will exceed 100 chars */
  struct Node *next;
  struct Node *prev;
};


/* prints the complete list */
void print(struct Node *node){
  struct Node *lastnode;

  /* find the last node */
  while (node!=NULL){
    lastnode=node;
    node = node->next;
  }

  /* Printing reverse order */
  //printf("\nPrinting words in reverse alphabetical order\n");
  while (lastnode!=NULL){
    printf(" %s\n",lastnode->data);
    lastnode = lastnode->prev;
  }
}

int compareStrings(char str1[],char str2[]){
  int i;
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int stringLength;
  if (len1<len2){
    stringLength = len1;
  } else{
    stringLength = len2;
  }
  for (i=0;i<stringLength;i++){
    if ((isupper(str1[i]) != 0) && (isupper(str2[i]) == 0)){
      return 1;
    } else if((isupper(str1[i]) == 0)&&(isupper(str2[i]) != 0)){
      return 2;
    } else{
      int ascii1 = str1[i];
      int ascii2 = str2[i];
      if (ascii1>ascii2){
        return 1;
      } else if (ascii1<ascii2){
        return 2;
      } else{
        continue;
      }
    }

  }
  if (len1 == len2){
    return 3;
  } else if (len1<len2){
    return 2;
  } else{
    return 1;
  }
  return 0;
}

struct Node* addToken(struct Node **head, char *str){

//  if (debug == 2)
//    printf("In addToken for %s\n", str);


  /* create the new node first */
  struct Node* new_node=(struct Node*)malloc(sizeof(struct Node));
  strcpy(new_node->data,str);
  new_node->prev = NULL;
  new_node->next = NULL;


  // This is the first time here
  if (*head==NULL){
    *head = new_node;
    return *head;
  }
  else
  {
    // we need to find where to inserted
    struct Node *node = *head;
    struct Node **prev = head;
    int first = 0;
    while (node){


      // go through the print_list
      int result = compareStrings(node->data, new_node->data);
//      printf("Compare: %d %s %s\n",result, node->data, new_node->data);
      if (result == 1||result==3)
      {
        if (first == 0) {
          new_node->next = node;
          *prev = new_node;
          node->prev = new_node;
          return *head;
        }
        else {
//          printf("In result=1 and first=1\n");
          new_node-> next = node;
          node->prev->next = new_node;
          new_node->prev = node->prev;
          node->prev = new_node;
          return *head;
        }
      }

      if (result == 2)
      {
        if (node->next == NULL){
          node->next = new_node;
          new_node->prev = node;
          return *head;
        }
//        else {
          // if the result is 2
//          printf("I am here");
  //        printf(" %s %s\n", node->data, new_node->data);
//          print(*head);
//        }
      }

      prev = &node;
      node = node->next;
      first = 1;
//      printf("result is %d\n",result);
    }
    return NULL;
  }

}






int main (int argc, char **argv)
{
  /* only one argument supported */
  if (argc != 2){
    int arguments = argc-1;
    printf("Error Found %d arguments, was expecting 1.\nUsage: ./pointersorter singleinput\n",arguments);
    exit(-1);
  }


  int i = 0;
  char array[100];

  struct Node *head = NULL;
  int begin = 0;
  int end = 0;
  int isword = 0;
  char temptoken[100];

  /* read until the end of the string */
  while (i < strlen(argv[1]) + 1)
  {

    char next = argv[1][i];
  //  if (debug==1){
  //    printf("char %c Begin: %d End: %d isword: %d\n", next, begin, end, isword);
  //  }
    /* first time check */

    if (!isalpha(next))
    {
        /* if we had started a word */
        if (isword == 1){
//          if (debug == 1)
  //          printf("We have a token char %c Begin: %d End: %d isword: %d\n", next, begin, end, isword);
          /* create word and set isword = 0 */
          isword = 0;
          int length = end - begin + 1;

  //        if (debug == 1){
//            printf("Length is %d\n", length);
//          }

          strncpy(temptoken,&argv[1][begin], end-begin+1);
          temptoken[length]='\0';
          begin = end+1;
//          if (debug == 1)
//            printf("Token is %s\n", temptoken);
          struct Node *temp = addToken(&head, temptoken);
//          print(head);
        }
      begin++;
    }
    else {
      /* this is an alphabet */
      /* if isword is not set, then set it */
      if (!isword){
        isword = 1;

        /* begin is now correct. Now set end to this */
        end = begin;
      }
      else {
        end = i;
      }
    }
    i++;
  }
  print(head);

}
