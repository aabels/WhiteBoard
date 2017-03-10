#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "encryptdecrypt.c"

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2
#define QUERY "find"
#define UPDATE "update"
#define CLEAN "clean"
#define QUIT_CMND "quit"

static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return OK;
}

int main( int argc, char* argv[]) {
  int rc, big_flag = 1;
  char buff[1024], tooServer[1024], type[2];

  while (big_flag){
    rc = getLine ("Enter...find | update | clean | quit: ", buff, sizeof(buff));
    if (rc == NO_INPUT) {
        printf ("No input\n");
        return 1;
    }

    if (rc == TOO_LONG) {
        printf ("Input too long\n");
        return 1;
    }
    if (strcmp(buff,QUIT_CMND)== 0){
      big_flag = 0;
      break;
    }
    if (strcmp(buff,QUERY)== 0) {
      memset(buff, 0, sizeof(buff));
      rc = getLine ("Enter specified entry number: ", buff, sizeof(buff));
      printf ("Entry number:  [%s]\n", buff);
      sprintf(tooServer, "?%s\n", buff);
      //send to the Server
      printf("%s\n", tooServer);
      //depending on if the entry is c or p do something HERE
      //ie. try to decrypt using the keys in a text file
    }
    if (strcmp(buff,UPDATE)== 0) {
      memset(buff, 0, sizeof(buff));
      rc = getLine ("Enter entry number to update: ", buff, sizeof(buff));
      printf ("Entry number:  [%s]\n", buff);
      sprintf(tooServer, "@%s", buff);

      rc = getLine ("(c or p): ", type, sizeof(type));
      sprintf(tooServer+strlen(tooServer),"%s",type);
      //depending on if its c or p encrypt or decrypt or just send to server
      printf("%s\n", tooServer);

      memset(buff, 0, sizeof(buff));
      rc = getLine ("Enter your message. ", buff, sizeof(buff));

      
      char c_char[1] = "c", p_char[1] = "p";
      if (strncmp(type, c_char, 1)==0){
        char* s = malloc(34);
        strcpy(s, "CMPUT379 Whiteboard Encrypted v0\n");
        prepend(buff, s);
        do_func(buff, 1);
        char *str1 = get_output();
        printf("Base-64 encoded string is: %s\n", str1);  //Prints base64 encoded string.
        sprintf(tooServer+ strlen(tooServer),"%lu\n%s\n", strlen(str1), str1);
        free(s);
      }
      else if (strncmp(type, p_char, 1)==0){
        sprintf(tooServer+ strlen(tooServer),"%lu\n%s\n", strlen(buff), buff);
      }
      memset(type, 0, sizeof(type));
      //send to server
      printf("%s\n", tooServer);

      char * handle[1];
      long int action[2];
      char command[1024];
      char *p;

      //command segment here
      handle[0] = strtok (tooServer, "\n");
      //message segment here
      handle[1] = strtok (NULL, "\n");
      printf("Handle[0]: %s Handle[1] %s\n", handle[0], handle[1]);

      //used to get the msg length and entry number as integers
      strcpy(command, handle[0]);
      if (strncmp(command, "@", 1)==0) {
        printf("This is a update\n");
      }
      if (strncmp(command, "?", 1)==0) {
        printf("This is a query\n");
      }
      printf("%s\n",command);
      p = command;
      int count = 0;
      while (*p) {
        if (isdigit(*p)) {
          long val = strtol(p, &p, 10);
          action[count] = val;
          printf("TEST\n");
          count++;
        }
        else {
          p++;
        }
      }
      printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);

    }
    if (strcmp(buff, CLEAN)== 0) {
      memset(buff, 0, sizeof(buff));
      memset(type, 0, sizeof(type));
      rc = getLine ("Enter entry number to clean: ", buff, sizeof(buff));
      sprintf(tooServer, "@%s", buff);
      //Does it matter if the entry was p or e to clean?
      //rc = getLine ("(c or p): ", type, sizeof(type));
      char p_char[1] = "p";
      strncpy(type, p_char, 1);
      sprintf(tooServer+strlen(tooServer),"%s0\n",type);
      memset(buff, 0, sizeof(buff));
      memset(type, 0, sizeof(type));
      //send to server
      printf("%s\n", tooServer);
    }
  }

  return 0;
}
//taken from
//http://stackoverflow.com/questions/4815672/handling-string-input-in-c
