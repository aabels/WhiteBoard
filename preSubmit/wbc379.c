#include "wbc379.h"

int main(int argc, char* argv[])
{
	
	int	rc, s, big_flag = 1, upon_entry = 1, setup_flag = 0;
	char msg[1024], buff[1024], tooServer[1024], type[2];
	char c_char[1] = "c", p_char[1] = "p";
	char * handle[1];
	long int action[2];
	char command[1024];
	char *p;
	char ip[1024];
	int port;

	struct	sockaddr_in	server;
	struct	hostent		*host;

	while (!setup_flag) {
		if (argc == 1) {
			printf("Default settings with default port and keyfile\n");
			host = gethostbyname ("localhost");
			port = MY_PORT;
			setup_flag = 1;
		}
		else if (argc == 2) {
			host = gethostbyname (argv[1]);
			exit(1);
		}
		else if (argc == 3) {
			host = gethostbyname (argv[1]);
			port = atoi(argv[2]);
			fp_keyfile = NULL;
			printf("Port is %d \n", port);
			printf("No keyfile! Encrypt and decrypt features will be unavailable\n");
			setup_flag = 1;
		}
		else if (argc == 4) {
			port = atoi(argv[2]);
			host = gethostbyname (argv[1]);
			printf("Port number is %d\n", port);

			FILE *fp_tester;
			fp_tester = fopen(argv[3], "r");
			if (fp_tester == NULL) {
				fp_keyfile = NULL;
				printf("Invalid keyfile! Encrypt and decrypt features will be unavailable\n");
			}
			else {
				printf("Key file %s\n", argv[3]);
				fp_keyfile = argv[3];
			}
			setup_flag = 1;		
		}
		else {
			printf("Something went wrong please re-type specifications.\n");
			exit(1);
		}
	}



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
	server.sin_port = htons (port);
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
	    if (strcmp(buff,QUERY)== 0) {
			rc = getLine ("Enter specified entry number: ", buff, sizeof(buff));
			sprintf(tooServer, "?%s\n", buff);
			send(s,tooServer, 100, 0);
			sleep(2);
			//Recieving the completed task
			memset(msg, 0, sizeof(char)*1024);
			memset(buff, 0, sizeof(char)*1024);
			bzero(buff, 1024);
			bzero(msg, 1024);
			recv(s, msg, 1024, 0);

			//NEED TO CHECK THE TYPE OF MSG HERE
			char uncrypted_msg[1024];
			char *temp_msg = (char *) msg;
			char * dec_input[1];
			dec_input[0] = strtok (temp_msg, "\n");
			dec_input[1] = strtok (NULL, "\n");
			char command_1[1024];
			memset(command_1, 0, sizeof(char)*1024);
			char p_entry[1024];
			strcpy(command_1, dec_input[0]);
			char * p_1;
			p_1 = command_1;
			
			while (*p_1) {
				if (isalpha(*p_1)) {
					strncpy(type, &(*p_1), 1);
				}
				p_1++;
			}
			//If the Query is Encrpted
			if (strncmp(type, c_char, 1)==0) {
				strcpy(buff, dec_input[1]);
			    do_func(buff, 2, fp_keyfile);
			    char *str1 = get_output();
			    printf("%s\n", str1);

			    char* str_to_cmp = malloc(33);
		        strcpy(str_to_cmp, "CMPUT379 Whiteboard Encrypted v0\n");

		        if (strncmp(str1, str_to_cmp, 33)!=0){
		        	printf("Client was unable to find correct key.\n");
		        }

			    bzero(p_entry, 0);
				bzero(buff, 0);
				bzero(msg, 0);
			}
			else {
				if (dec_input[1] == NULL) {
					printf("%s\n", msg);
				}
				else {
					strcpy(uncrypted_msg, dec_input[1]);
					sprintf(p_entry+strlen(p_entry),"%s\n%s",command_1, uncrypted_msg);
					printf("%s\n", p_entry);
					bzero(p_entry, 0);
					bzero(buff, 0);
					bzero(msg, 0);
						
				}
			}
	    }
    	else if (strcmp(buff,UPDATE)== 0) {
	      	memset(buff, 0, sizeof(buff));
	      	rc = getLine ("Enter entry number to update: ", buff, sizeof(buff));
			
			sprintf(tooServer, "@%s", buff);
			rc = getLine ("(c or p): ", type, sizeof(type));
			sprintf(tooServer+strlen(tooServer),"%s",type);
			//depending on if its c or p encrypt or decrypt or just send to server
			printf("%s\n", tooServer);
			memset(buff, 0, sizeof(buff));
			rc = getLine ("Enter your message: ", buff, sizeof(buff));

			if (strncmp(type, c_char, 1)==0) {

				if (fp_keyfile == NULL) {
					printf("Unable to encrypt! Features are unavailable.\n");
				}
				else {
			        char* not_s = malloc(34);
			        strcpy(not_s, "CMPUT379 Whiteboard Encrypted v0\n");
			        prepend(buff, not_s);
			        do_func(buff, 1, fp_keyfile);
			        char *str1 = get_output();
			        sprintf(tooServer+ strlen(tooServer),"%lu\n%s\n", strlen(str1), str1);
			        send(s,tooServer, 1024, 0);
					bzero(buff, 1024);
					bzero(msg, 1024);
					recv(s, msg, 1024, 0);
					printf("%s\n", msg);
			    	memset(msg, 0, sizeof(msg));
					memset(buff, 0, sizeof(buff));
			        free(not_s);
				}
	      	}
	      	else if (strncmp(type, p_char, 1)==0){
	        	sprintf(tooServer+ strlen(tooServer),"%lu\n%s\n", strlen(buff), buff);
	        	send(s,tooServer, 100, 0);

	        	bzero(buff, 1024);
				bzero(msg, 1024);
				recv(s, msg, 1024, 0);
				printf("%s\n", msg);
		    	memset(msg, 0, sizeof(msg));
				memset(buff, 0, sizeof(buff));
				
	      	}
	      	memset(type, 0, sizeof(type));
			//command segment here
			handle[0] = strtok (tooServer, "\n");
			if (handle[0] == NULL) {
				printf("Please Re-Enter your Update MSG\n");
			}
			else {
				//message segment here
				handle[1] = strtok (NULL, "\n");
		      	//used to get the msg length and entry number as integers
		      	strcpy(command, handle[0]);
		      	p = command;
		      	int count = 0;
		      	while (*p) {
		        	if (isdigit(*p)) {
		          		long val = strtol(p, &p, 10);
		          		action[count] = val;
		          		count++;
		        	}
		        	else {
		          		p++;
		        	}
		      	}
			}
			memset(tooServer, 0, sizeof(tooServer));
      	}
	    else if (strcmp(buff, CLEAN)== 0) {
			memset(buff, 0, sizeof(buff));
			memset(type, 0, sizeof(type));
			rc = getLine ("Enter entry number to clean: ", buff, sizeof(buff));
			sprintf(tooServer, "@%s", buff);
			char p_char[1] = "p";
			strncpy(type, p_char, 1);
			sprintf(tooServer+strlen(tooServer),"%s0\n",type);
			memset(buff, 0, sizeof(buff));
			memset(type, 0, sizeof(type));
			//send to server
			send(s,tooServer, 100, 0);
			bzero(buff, 1024);
			bzero(msg, 1024);
			//recieve back
			recv(s, msg, 1024, 0);
			printf("%s\n", msg);
		    memset(msg, 0, sizeof(msg));
			memset(buff, 0, sizeof(buff));
			bzero(tooServer, 0);
	    }	
		memset(buff, 0, sizeof(buff));
		memset(type, 0, sizeof(type));
		memset(command, 0, sizeof(command));	
	}
	close(s);
	exit(1);
}