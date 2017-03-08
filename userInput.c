#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2
#define QUERY "find"
#define UPDATE "update"
#define CLEAN "clean"

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
  int rc;
  char buff[100], tooServer[1024], type[2];

  rc = getLine ("Enter...find | update | clean: ", buff, sizeof(buff));
  if (rc == NO_INPUT) {
      printf ("No input\n");
      return 1;
  }

  if (rc == TOO_LONG) {
      printf ("Input too long\n");
      return 1;
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
    memset(type, 0, sizeof(type));
    rc = getLine ("Enter your message. ", buff, sizeof(buff));
    sprintf(tooServer+ strlen(tooServer),"%d\n%s\n", strlen(buff), buff);
    //send to server
    printf("%s\n", tooServer);
  }
  if (strcmp(buff, CLEAN)== 0) {
    memset(buff, 0, sizeof(buff));
    memset(type, 0, sizeof(type));
    rc = getLine ("Enter entry number to clean: ", buff, sizeof(buff));
    sprintf(tooServer, "@%s", buff);
    //Does it matter if the entry was p or e to clean?
    rc = getLine ("(c or p): ", type, sizeof(type));
    sprintf(tooServer+strlen(tooServer),"%s\n\n",type);
    memset(buff, 0, sizeof(buff));
    memset(type, 0, sizeof(type));
    //send to server
    printf("%s\n", tooServer);
  }

  return 0;
}

//taken from
//http://stackoverflow.com/questions/4815672/handling-string-input-in-c
