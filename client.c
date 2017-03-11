#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "encryptdecrypt.c"

#define	 MY_PORT  2229
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2
#define QUERY 		"find"
#define QUIT_CMND 	"quit"
#define CLEAN 		"clean"
#define UPDATE 		"update"

static int getLine (char *prmpt, char *buff, size_t sz);

int main()
{
	
	int	rc, s, big_flag = 1, upon_entry = 1;
	char msg[1024], buff[102], tooServer[1024], type[2];
	//char * Fupdate = "?12p\n";
	char c_char[1] = "c", p_char[1] = "p";
	char * handle[1];
	long int action[2];
	char command[1024];
	char *p;
	//update:"@12p4\ncool\n";

	struct	sockaddr_in	server;
	struct	hostent		*host;

	host = gethostbyname ("localhost");

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}
	//server connecting intialization
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}
	bzero (&server, sizeof (server));
	bcopy (host->h_addr, & (server.sin_addr), host->h_length);
	server.sin_family = host->h_addrtype;
	server.sin_port = htons (MY_PORT);
	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}

	while (big_flag) {
		//Recieving First HandShake Here
		if (upon_entry) {
			bzero(msg, 1024);
			recv(s, msg, 1024, 0);
			printf("%s\n", msg);
			bzero(msg, 1024);
			sleep (2);
			upon_entry = 0;			
		}
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
		//printf("sending: %s \n", Fupdate);
	    if (strcmp(buff,QUERY)== 0) {
			rc = getLine ("Enter specified entry number: ", buff, sizeof(buff));
			//printf ("Entry number:  [%s]\n", buff);
			sprintf(tooServer, "?%s\n", buff);
			//printf("tooServer2:%s\n", tooServer);
			send(s,tooServer, 100, 0);
			sleep(2);
			//Recieving the completed task
			bzero(buff, 100);
			bzero(msg, 1024);
			recv(s, msg, 1024, 0);
			printf("%s\n", msg);
	    	memset(msg, 0, sizeof(msg));
			memset(buff, 0, sizeof(buff));
			memset(tooServer, 0, sizeof(tooServer));
			//depending on if the entry is c or p do something HERE
			//ie. try to decrypt using the keys in a text file
	    }
    	else if (strcmp(buff,UPDATE)== 0) {
	      	memset(buff, 0, sizeof(buff));
	      	rc = getLine ("Enter entry number to update: ", buff, sizeof(buff));
			//printf ("Entry number:  [%s]\n", buff);
			sprintf(tooServer, "@%s", buff);
			rc = getLine ("(c or p): ", type, sizeof(type));
			sprintf(tooServer+strlen(tooServer),"%s",type);
			//depending on if its c or p encrypt or decrypt or just send to server
			printf("%s\n", tooServer);
			memset(buff, 0, sizeof(buff));
			rc = getLine ("Enter your message. ", buff, sizeof(buff));

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
	        	send(s,tooServer, 100, 0);
	      	}
	      	memset(type, 0, sizeof(type));
	      	//send to server
	      	//printf("%s", tooServer)
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
	    else if (strcmp(buff, CLEAN)== 0) {
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
			bzero(tooServer, 0);
			// memset(tooServer, 0, sizeof(tooServer));
			//send to server
			printf("%s\n", tooServer);
	    }
		//Sending Query Here
		// send(s,Fupdate, 100, 0);
		// sleep(2);

		// //Recieving the completed task
		// bzero(msg, 1024);
		// send(s,Fupdate, 100, 0);
		// recv(s, msg, 1024, 0);
		// printf("%s\n", msg);
		// sleep (2);
		memset(buff, 0, sizeof(buff));
		memset(type, 0, sizeof(type));
		memset(command, 0, sizeof(command));	
		//fprintf (stderr, "Process %d gets number %d\n", getpid (), ntohl (number));
	}
		close (s);
		exit(1);
}

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