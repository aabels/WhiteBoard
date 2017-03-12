#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define	MY_PORT	2229
#define ENTRY_SIZE 25
#define NUM_THREADS 1

pthread_t thread[NUM_THREADS];
int entryCAP = 0;
unsigned int snew;
char ** entry;
char ** det;

FILE* statefile;
void init_board(FILE *statefile);
void free_board (char ** wb);
int fib (int n);
void * handle_client (void * snew);
void * process_task (void * socket, char * prompt);
long ** cmd_parse (char * string);

void * process_task (void * socket, char * prompt) {
	int connection = *(int *) socket;
	int index = 0;
	char * task;
	char const empty_msg[] = "Empty\n";
	char const ranng_msg[] = "That entry number does not exist\n";
	char const suc_msg[] = "!";
	char * handle[1];
	char * sendback[1];
    long int action[2];
    long int action2[2];
    char command[1024], query[1024], result[1024], error_msg[1024];
    char *p;
    int count = 0, flag =0;
	char type[2], newtype[2];
	//fib(30);
    task = (char *) prompt;
    //command segment here
    handle[0] = strtok (task, "\n");
    //message segment here
    handle[1] = strtok (NULL, "\n");
    //meaning if its not a query grab the type, len, and entry #
    strcpy(command, handle[0]);
	//printf("Do I go in\n");
	p = command;
	action[0] = 0;
	action[1] = 0;
    printf("Handle[0]: %s Handle[1] %s\n", handle[0], handle[1]);
    
    if (handle[1] != NULL){
	    while (*p) {
			if (isdigit(*p)) {
				long val = strtol(p, &p, 10);
				action[count] = val;
				count++;
			}
			if (isalpha(*p)) {
				flag = 1;
				if (flag){
					printf("test\n");
					strncpy(type, &(*p), 1);
					flag = 0;
				}
			}
			p++;
		}
	}

    //used to get the msg length and entry number as integers
	//check if its a query '?' or '@'
	if (strncmp(command, "?", 1)==0) {
		while (*p) {
			if (isdigit(*p)) {
				long val = strtol(p, &p, 10);
				action[count] = val;
				count++;
			}
			if (isalpha(*p)) {
				flag = 1;
				if (flag){
					printf("test\n");
					strncpy(type, &(*p), 1);
					flag = 0;
				}
			}
			p++;
		}
		if (action[0]==0){
			printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);
			send(connection, ranng_msg, 32, 0);
	    	return (void *) 0;
		}
    	action[0] = action[0] - 1;
		//check if the entry exists (is valid) if not return error to client
		printf("action[0]: %ld action[1]: %ld, type %s\n", action[0], action[1], det[action[0]]);
		index = (int) action[0];
    	memcpy((void *)query, (const void *)entry[index], sizeof(char)*1000);

    	if (strlen(entry[index]) == 0) {
    		printf("test\n");
    		bzero(error_msg, 0);
    		sprintf(error_msg+strlen(error_msg), "!%lup0\n",(action[0]+1));
    		printf("too server %s\n", error_msg);
    		send(connection, error_msg, 40, 0);
    		memset(error_msg, 0, 1024);
    		return (void *) 0;
    	}
    	if (index > entryCAP){
    		send(connection, ranng_msg, 32, 0);
    		return (void *) 0;
    	}
    	//tolucheckpoint
    	//strcpy(sendback[0], entry[11]);
    	printf("Query is: %s\n", query);
	
    	printf("action[0]: %ld action[1]: %ld, type %s\n", action[0], action[1], det[action[0]]);
    	// printf("This is a query of type %s \n", newtype);
    	//to accomodate the entry numbers starting at 1
    	//locate query here with wb[index] and send results to client
    	//using the size of result bc no extra allocation implimented yet
    	sprintf(result+strlen(result), "!%ld%s%ld\n", (action[0]+1), type, action[1]);
    	sprintf(result+strlen(result), "%s\n", query);
    	// printf("this is the query %s\n", sendback[1]);
    	printf("query is: %s and result is: %s\n", query, result);
    	send(connection, result, 1024, 0);
    	memset(result, 0, sizeof(result));
    	memset(query, 0, sizeof(query));
    	fib(30);
    	sleep(2);
    }
    else if (strncmp(command, "@", 1)==0) {
    	if (action[0]==0 || action[0] > entryCAP){
			printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);
			send(connection, ranng_msg, 32, 0);
	    	return (void *) 0;
		}
    	//strcpy(command, handle[1]);
    	//need to check if its empty
    	action[0] = action[0] - 1;
    	
		//"11p14thisIsASercret\n\n
		//check if the entry exists (is valid) if not return error to client
		index = (int) action[0];
    	if (index > entryCAP) {
    		printf("here\n");
    		send(connection, ranng_msg, 32, 0);
    		return (void *) 0;

    	}
    	det[index] = type;
		printf("action[0]: %ld action[1]: %ld, type %s\n", action[0], action[1], det[index]);
    	memcpy((void *)query, (const void *)entry[index], sizeof(char)*action[1]);
    	if (strlen(query)== 0) {
    		//store the new entry immediatly and keep track of the type[index]
    		memset(entry[index], 0, sizeof(char)*50);
    		memcpy((void *)entry[index], (const void *)handle[1], sizeof(char)*action[1]);
    		//entry[index] = task;
    		printf(" Index %d Entry %d is: %s\n", index, index, entry[index]);
    		printf("success\n");
    		sprintf(error_msg+strlen(error_msg), "%s%lue0", suc_msg, (action[0]+1));
    		send(connection, error_msg, 100, 0);
    		bzero(error_msg, 0);
    	}
    	else {
    		memset(entry[index], 0, sizeof(char)*50);
    		memcpy((void *)entry[index], (const void *)handle[1], sizeof(char)*action[1]);
    		printf(" Index %d Entry %d is: %s\n", index, index, entry[index]);
    		printf("success\n");
    		memset(error_msg, 0, sizeof(error_msg));

    		sprintf(error_msg+strlen(error_msg), "%s%lue0", suc_msg, (action[0]+1));
    		send(connection, error_msg, 100, 0);
    	}
    }
    	//if not we need to clean the entry
    	//store new entry
  	else {
    	//return an error to the client here
    	printf("A syntactical error occured\n");
    }
    	return (void *) 0;
}

int fib (int n){
    if ((n == 0) | (n == 1))
        return 1;
    return fib(n-1) + fib(n-2);
}

void * handle_client (void * snew) {
	//how do you pass connection into thread func
	//upon entry server responds with msg and # of entries
	int socket = *(int *) snew;
	char prompt[10000];
	sprintf(prompt,"CMPUT379 Whiteboard Server v0\n%d\n",entryCAP);
	send(socket,prompt, 100, 0);
	sleep(1);

	//I want to recieve a command here
	for (;;) {
		bzero(prompt, 10000);
		recv(socket, prompt, sizeof(prompt), 0);
		printf("test, value of prompt: %s\n", prompt);
		//printf("value of prompt %s\n", prompt);
		if (prompt != NULL){
			process_task( (void*) &socket, (char*) prompt);
		}
		
	}
	pthread_exit(0);
	close(socket);

	return (void *) 0;
}

int main(int argc, char* argv[]){

	int socky, snew, fromlength, open_e, settings_flag;							//129.128.41.20

	char * options[1];

	//-f (statefile) | -n (# of entries)
	options[0] = "-f";
	options[1] = "-n";
	//do we need a count of used entries?
	open_e = 0;
	settings_flag = 0;

	struct sockaddr_in master, from;
	while (!settings_flag) {
		if (argc == 1) {
			//set default -n entry wb and default portnumber
			printf("Default settings\n");
			entryCAP = ENTRY_SIZE;
			statefile = NULL;
			init_board(statefile);
			settings_flag = 1;
		}
		else if (argc == 2) {
			printf("Default with specified portnumber\n");
			settings_flag = 1;
		}
		else if (argc == 3) {
			printf("This is argv[2]: %s. And this is argv[3] %s\n", argv[2], argv[3] );

			if (strcmp(argv[2], options[0]) == 0) {
				//enter file name for preloaded wb
				printf("Error need to specify a file to be loaded.\n");
				settings_flag = 1;
			}
			else if (strcmp(argv[2], options[1]) == 0) {
				//enter entry size of wb
				entryCAP = ENTRY_SIZE;
				printf("Wb entry max size.%d\n", entryCAP);
				settings_flag = 1;
			}
			else if (argv[3] == NULL){
				printf("Default with specified portnumber\n");
				settings_flag = 1;
				}
			}
		else if (argc == 4) {

			if (strcmp(argv[2], options[0]) == 0) {
				printf("Pre-loaded statefile %s\n", argv[3]);
				statefile = fopen(argv[3], "w+");
				settings_flag = 1;
			}
			else if (strcmp(argv[2], options[1]) == 0) {
				entryCAP = atof(argv[3]);
				if (entryCAP > 500) {
					entryCAP = ENTRY_SIZE;
				}
				printf("Wb entry max size.%d\n", entryCAP);
				settings_flag = 1;
				}
		}
		else {
			printf("Something went wrong please re-type specifications.\n");
			exit(1);
		}
	}



	//creates an endpoint for communication and return a file descriptor = socky
	socky = socket (AF_INET, SOCK_STREAM, 0);//AFI_NET (IPv4) protocol family, SOCK_STREAM (reliabletwo way connection)
	int enable =1;
	if (setsockopt(socky, SOL_SOCKET, SO_REUSEADDR,&enable,sizeof(int))<0){
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	else if (socky < 0) {
		perror ("Sever: cannot open the master socket");
		exit(1);
	}

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (MY_PORT);

	//create a socket and associate local address and port with the socket
	if (bind (socky, (struct sockaddr *) &master, sizeof (master))) { //returns 0 on success -1 on error
		perror ("Server: cannot bind master socket");
		exit(1);
	}

	//marks socky as a passive socket ready to accept incoming connection requests
	listen (socky, 7); //7 is the backlog number to limit the number of outstanding connection in the listen queue

	for (;;) {
		fromlength = sizeof (from);
		snew = accept (socky, (struct sockaddr*) & from, (socklen_t*)&fromlength);
		if (snew < 0) {
			perror ("Server: accept failed");
			exit (1);
		}

		//a pthread is created for each new Client
		pthread_create(&thread[open_e], NULL, handle_client, (void *) &snew);
		open_e++;
	}


}


void init_board (FILE * input) {
	int i,k;
	//means we are starting from a blank WB
	if (input == NULL) {
		//printf("Entry Cap = %d\n", entryCAP);
		entry = (char **) malloc (entryCAP*sizeof(char*));
		det = (char **) malloc (entryCAP*sizeof(char*));
		for (i=0; i <= entryCAP-1; i++) {
			entry[i] = (char *) malloc (1024*sizeof(char));
			det[i] = (char *) malloc (1*sizeof(char));
		}
		statefile = fopen("statefile.txt", "w+");
		for (k=0; k <= entryCAP -1; k++) {
			det[k] = "p";
			fprintf(statefile, "!%dp%lu\n%s\n",k, strlen(entry[k]), entry[k]);
			//printf("Entry %d %s\n", k, entry[k]);
		}
		fclose(statefile);
	}
}

void free_board (char ** wb) {
	int j;
	for (j=0; j <= entryCAP -1; j++) {
		free(entry[j]);
	}
	free(entry);
}

long ** cmd_parse (char * string) {
	int counter=0;
	long ** bundle;
	long value;
	char * str;
	str = string;
	while (*str) {
		if (isdigit(*str)) {
			long value = strtol(str, &str, 10);
			bundle[counter] = (long *) value;
		}
		str++;
	}
	return bundle;
}









/*websites for notes:
https://linux.die.net/man/2/socket
https://linux.die.net/man/7/socket
https://linux.die.net/man/2/connect
https://linux.die.net/man/2/listen
https://linux.die.net/man/3/listen
https://linux.die.net/man/2/accept
https://linux.die.net/man/3/accept
*/