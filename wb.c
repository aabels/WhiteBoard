#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	MY_PORT	2224
#define ENTRY_SIZE 25
int entryCAP = 0;
char ** entry;
FILE* statefile;

int main(int argc, char* argv[]){

	int socky, snew, fromlength, open_e, settings_flag, entry_flag;							//129.128.41.20
	char prompt[100], a_word[ENTRY_SIZE];
	char * options[1];

	//-f (statefile) | -n (# of entries)
	options[0] = "-f";
	options[1] = "-n";
	open_e = 0;
	settings_flag = 0;
	entry_flag = 0;

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
		snew = accept (socky, (struct sockaddr*) & from, & fromlength);
		if (snew < 0) {
			perror ("Server: accept failed");
			exit (1);
		}
		//upon entry server responds with msg and # of entries
		else if (!entry_flag) {
			sprintf(prompt,"CMPUT379 Whiteboard Server v0\\n%d",entryCAP);
			//printf ("%s\n", prompt);
			send(snew,prompt, 100, 0);
			entry_flag = 1;
			close(snew);
			sleep(1);		
		}
	}

	
}


void init_board (FILE * input) {
	int i,k;
	//means we are starting from a blank WB
	if (input == NULL) {
		//printf("Entry Cap = %d\n", entryCAP);
		entry = (char *) malloc (entryCAP*sizeof(char*));
		for (i=0; i <= entryCAP-1; i++) {
			entry[i] = (char *) malloc (100*sizeof(char));
		}
		statefile = fopen("statefile.txt", "w+");
		for (k=0; k <= entryCAP -1; k++) {
			fprintf(statefile, "Entry[%d] : contents: %s\n", k, entry[k]);
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