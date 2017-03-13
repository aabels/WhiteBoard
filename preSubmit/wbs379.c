#include "wbs379.h"

int main(int argc, char* argv[]){

	signal(SIGTERM, sighandle_sigterm);

	int socky, snew, fromlength, open_e, settings_flag;
	int portnumber;							//129.128.41.20

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
			//statefile = NULL;
			//statefile = fopen("statefile.txt", "w+");
			settings_flag = 1;
			portnumber = MY_PORT;
		}
		else if (argc == 2) {
			portnumber = atoi(argv[1]);
			printf("port number is %d\n", portnumber);
			settings_flag = 1;
		}
		else if (argc == 3) {
			//printf("This is argv[2]: %s. And this is argv[3] %s\n", argv[2], argv[3] );

			if (strcmp(argv[2], options[0]) == 0) {
				//enter file name for preloaded wb
				printf("Error need to specify the statefile to load \n");
			}
			else if (strcmp(argv[2], options[1]) == 0) {
				//enter entry size of wb
				entryCAP = ENTRY_SIZE;
				printf("Error need to specify number of entries \n");
			}
			exit(1);
		}
		else if (argc == 4) {
			portnumber = atoi(argv[1]);
			printf("port number is %d\n", portnumber);
			if (strcmp(argv[2], options[0]) == 0) {
				printf("Pre-loaded statefile %s\n", argv[3]);
				statefile = argv[3];
				read_statefile(statefile);
				settings_flag = 1;
			}
			else if (strcmp(argv[2], options[1]) == 0) {
				entryCAP = atoi(argv[3]);
				if (entryCAP > 500) {
					entryCAP = ENTRY_SIZE;
					init_board(statefile);
				}
				printf("Wb entry max size.%d\n", entryCAP);
				init_board(statefile);
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
	master.sin_port = htons (portnumber);

	//create a socket and associate local address and port with the socket
	if (bind (socky, (struct sockaddr *) &master, sizeof (master))) { //returns 0 on success -1 on error
		perror ("Server: cannot bind master socket");
		exit(1);
	}

	init_board(statefile);
    deamonize(deamonfile);

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
	fclose(deamonfile);
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