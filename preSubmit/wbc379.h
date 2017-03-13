#ifndef WBC379_H_   /* Include guard */
#define WBC379_H_

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "wbc379_ed.c"

#define	 MY_PORT  	2230
#define OK       	0
#define NO_INPUT 	1
#define TOO_LONG 	2
#define QUERY 		"find"
#define QUIT_CMND 	"quit"
#define CLEAN 		"clean"
#define UPDATE 		"update"

static int getLine (char *prmpt, char *buff, size_t sz);

char *fp_keyfile;

//http://stackoverflow.com/questions/4815672/handling-string-input-in-c
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

#endif // WBC379_H_