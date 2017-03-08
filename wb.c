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

#define	MY_PORT	2224
#define ENTRY_SIZE 25
#define NUM_THREADS 25

pthread_t thread[NUM_THREADS];
int id[NUM_THREADS];
int entryCAP = 0;
unsigned int snew;
char ** entry;
FILE* statefile;
void init_board(FILE *statefile);

int fib(int n){
    if ((n == 0) | (n == 1))
        return 1;
    return fib(n-1) + fib(n-2);
}

void * handle_client (void * snew) {
	//how do you pass connection into thread func
	//upon entry server responds with msg and # of entries
	int socket = *(int *) snew;
	char prompt[100];
	fib(30);
	sprintf(prompt,"CMPUT379 Whiteboard Server v0\\n%d\n",entryCAP);
	send(socket,prompt, 100, 0);
	close(socket);
	sleep(1);

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

	//crate a socket and associate local address and port with the socket
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
		for (i=0; i <= entryCAP-1; i++) {
			entry[i] = (char *) malloc (100*sizeof(char));
		}
		statefile = fopen("statefile.txt", "w+");
		for (k=0; k <= entryCAP -1; k++) {
			fprintf(statefile, "!%dp%d\n%s\n",k, strlen(entry[k]), entry[k]);
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









/*websites for notes:
https://linux.die.net/man/2/socket
https://linux.die.net/man/7/socket
https://linux.die.net/man/2/connect
https://linux.die.net/man/2/listen
https://linux.die.net/man/3/listen
https://linux.die.net/man/2/accept
https://linux.die.net/man/3/accept
*/
