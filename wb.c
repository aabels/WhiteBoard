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
#include <signal.h>

#define	MY_PORT	2230
#define ENTRY_SIZE 25
#define NUM_THREADS 100

//kill -15 pid *********************

pthread_t thread[NUM_THREADS];
pthread_mutex_t mutex;
int entryCAP = 0;
unsigned int snew;
char ** entry;
char ** det;

FILE * deamonfile = NULL;
FILE * statefile;

void init_board(FILE *statefile);
void free_board ();
int fib (int n);
void * handle_client (void * snew);
void * process_task (void * socket, char * prompt);
long ** cmd_parse (char * string);
void save_wb();
void read_statefile(char *statefile_tmp);
void sighandle_sigterm(int sig);
void deamonize (FILE * deamonfile);


void * process_task (void * socket, char * prompt) {
	int connection = *(int *) socket;
	int index = 0;
	char * task;
	char const empty_msg[] = "Empty\n";
	char const ranng_msg[] = "That entry number does not exist\n";
	char const suc_msg[] = "!";
	char * handle[1];
	//char * sendback[1];
    long int action[2];
    char command[1024], query[1024], result[1024], error_msg[1024];
    char *p;
    int count = 0, flag =0;
	char type[2];
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
    //printf("Handle[0]: %s Handle[1] %s\n", handle[0], handle[1]);
    
    while (*p) {
		if (isdigit(*p)) {
			long val = strtol(p, &p, 10);
			action[count] = val;
			count++;
		}
		if (isalpha(*p)) {
			flag = 1;
			if (flag){
				//printf("test\n");
				strncpy(type, &(*p), 1);
				flag = 0;
			}
		}
		p++;
	}
	//printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);

    //used to get the msg length and entry number as integers
	//check if its a query '?' or '@'
	if (strncmp(command, "?", 1)==0) {
		while (*p) {
			if (isdigit(*p)) {
				long val = strtol(p, &p, 10);
				action[count] = val;
				//printf("ACtion count %d \n", action[count]);
				count++;
			}
			if (isalpha(*p)) {
				flag = 1;
				if (flag){
					//printf("test\n");
					strncpy(type, &(*p), 1);
					flag = 0;
				}
			}
			p++;
		}
		//printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);
		//Error handling for asking for entry zero
		if (action[0]==0){
			bzero(error_msg, 0);
			sprintf(error_msg+strlen(error_msg), "!%lue34\n%s", action[0], ranng_msg);
			send(connection, error_msg, 40, 0);
			memset(error_msg, 0, 1024);
	    	return (void *) 0;
		}
    	action[0] = action[0] - 1;
		index = (int) action[0];
		//check if the entry exists (is valid) if not return error to client
    	if (index >= entryCAP){
			bzero(error_msg, 0);
			sprintf(error_msg+strlen(error_msg), "!%lue34\n%s", (action[0]+1), ranng_msg);
			send(connection, error_msg, 40, 0);
			memset(error_msg, 0, 1024);
    		return (void *) 0;
    	}
		//printf("Do we go here\n");
		//printf("Entry %d Query is: %s\n", entry[index], query);
		//printf("action[0]: %ld action[1]: %ld, type %s\n", action[0], action[1], det[action[0]]);
    	memcpy((void *)query, (const void *)entry[index], sizeof(char)*1024);

    	//Error handling for when the entry is empty
    	if (strlen(entry[index]) == 0) {
    		//printf("test\n");
    		bzero(error_msg, 0);
    		sprintf(error_msg+strlen(error_msg), "!%lup0\n",(action[0]+1));
    		//printf("too server %s\n", error_msg);
    		send(connection, error_msg, 40, 0);
    		memset(error_msg, 0, 1024);
    		return (void *) 0;
    	}

    	if (strcmp(query, entry[index])==0) {
    		printf("Do i go here\n");
    		//NEEEED TO PARSE THE PRELOADED STATE FILE TO SEND RIGHT INDEX BACK
	  //   	handle[0] = strtok (query, "\n");
		 //    //message segment here
		 //    handle[1] = strtok (NULL, "\n");
		 //    //meaning if its not a query grab the type, len, and entry #
		 //    strcpy(command, handle[0]);
		 //    strcpy(query, handle[1]);
			// //printf("Do I go in\n");
			// p = command;

   //  		while (*p) {
			// if (isdigit(*p)) {
			// 	long val = strtol(p, &p, 10);
			// 	action[count] = val;
			// 	printf("ACtion count %d \n", action[count]);
			// 	count++;
			// }
			// if (isalpha(*p)) {
			// 	flag = 1;
			// 	if (flag){
			// 		printf("test\n");
			// 		strncpy(type, &(*p), 1);
			// 		flag = 0;
			// 	}
			// }
			// p++;
			// }

   //  		memset(result, 0, sizeof(result));
			// sprintf(result+strlen(result), "!%ld%s%ld\n", (action[0]+1), type, action[1]);
	  //   	sprintf(result+strlen(result), "%s", query);
    		memcpy(result, query, sizeof(char)*1024);
    		send(connection, result, 1024, 0);
    		memset(query, 0, sizeof(query));
    		memset(result, 0, sizeof(result));
    	}
    	else{

	    	//tolucheckpoint
	    	//strcpy(sendback[0], entry[11]);
	    	printf("Query is: %s\n", query);
	    	//printf("action[0]: %ld action[1]: %ld, type %s\n", action[0], action[1], det[action[0]]);
	    	// printf("This is a query of type %s \n", newtype);
	    	//to accomodate the entry numbers starting at 1
	    	//locate query here with wb[index] and send results to client
	    	//using the size of result bc no extra allocation implimented yet
	    	sprintf(result+strlen(result), "!%ld%s%ld\n", (action[0]+1), type, action[1]);
	    	sprintf(result+strlen(result), "%s", query);
	    	// printf("this is the query %s\n", sendback[1]);
	    	printf("query is: %s and result is: %s\n", query, result);
	    	send(connection, result, 1024, 0);
	    	memset(result, 0, sizeof(result));
	    	memset(query, 0, sizeof(query));
    	}
    }
    else if (strncmp(command, "@", 1)==0) {
  //   	printf("DO I GO HERE\n");
		// printf("action[0]: %ld action[1]: %ld", action[0], action[1]);

		//HANDELS CLEARING HERE
    	if ((action[0]==0  && action[1]==0)) {
    		action[0] = action[0] - 1;
    		index = (int) action[0];
    		if (index >= entryCAP || index == -1) {
	    		bzero(error_msg, 0);
				sprintf(error_msg+strlen(error_msg), "!%lue34\n%s", action[0], ranng_msg);
				send(connection, error_msg, 40, 0);
				memset(error_msg, 0, 1024);
	    		return (void *) 0;
    		}
    		else {
    			det[index] = "p";
    			memset(entry[index], '\0', sizeof(char)* 1024);
    			//printf("entry is %s and det[%d] %s\n", entry[index], index, det[index]);
    			memset(error_msg, 0, 1024);
    			return (void *) 0;
    		}

    	}
    	if (action[0]==0){
			//printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);
			bzero(error_msg, 0);
			memset(error_msg, 0, 1024);
			sprintf(error_msg+strlen(error_msg), "!%lue34\n%s", action[0], ranng_msg);
			send(connection, error_msg, 40, 0);
			memset(error_msg, 0, 1024);
	    	return (void *) 0;
		}
    	//strcpy(command, handle[1]);
    	//need to check if its empty
    	action[0] = action[0] - 1;
    	
		//"11p14thisIsASercret\n\n
		//check if the entry exists (is valid) if not return error to client
		index = (int) action[0];
    	if (index >= entryCAP) {
    		bzero(error_msg, 0);
			sprintf(error_msg+strlen(error_msg), "!%lue34\n%s", action[0], ranng_msg);
			send(connection, error_msg, 40, 0);
			memset(error_msg, 0, 1024);
    		return (void *) 0;

    	}
    	det[index] = type;
		//printf("action[0]: %ld action[1]: %ld, type %s\n", action[0], action[1], det[index]);
    	memcpy((void *)query, (const void *)entry[index], sizeof(char)*action[1]);
    	if (strlen(query)== 0) {
    		//store the new entry immediatly and keep track of the type[index]
    		memset(entry[index], 0, sizeof(char)*action[1]);
    		memset(error_msg, 0, 1024);
    		memcpy((void *)entry[index], (const void *)handle[1], sizeof(char)*action[1]);
    		//entry[index] = task;
    		//printf(" Index %d Entry %d is: %s\n", index, index, entry[index]);
    		//printf("success\n");
    		sprintf(error_msg+strlen(error_msg), "%s%lue0", suc_msg, (action[0]+1));
    		send(connection, error_msg, 100, 0);
    		bzero(error_msg, 0);
    		memset(error_msg, 0, 1024);
    	}
    	else {
    		memset(entry[index], 0, sizeof(char)*action[1]);
    		memset(error_msg, 0, 1024);
    		memcpy((void *)entry[index], (const void *)handle[1], sizeof(char)*action[1]);
    		//printf(" Index %d Entry %d is: %s\n", index, index, entry[index]);
    		//printf("success\n");
    		memset(error_msg, 0, sizeof(error_msg));
    		sprintf(error_msg+strlen(error_msg), "%s%lue0", suc_msg, (action[0]+1));
    		send(connection, error_msg, 100, 0);
    		memset(error_msg, 0, 1024);
    	}
    }
  	else {
    	//return an error to the client here
    	printf("A syntactical error occured\n");
    }
    	//printf("End reacched\n");
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
	char prompt[1024];
	sprintf(prompt,"CMPUT379 Whiteboard Server v0\n%d\n",entryCAP);
	send(socket,prompt, 100, 0);
	sleep(1);

	//I want to recieve a command here
	for (;;) {
		bzero(prompt, 1024);
		recv(socket, prompt, sizeof(prompt), 0);
		//printf("test, value of prompt: %s\n", prompt);
		//printf("value of prompt %s\n", prompt);
		if (prompt != NULL){
			pthread_mutex_lock(&mutex);
			process_task( (void*) &socket, (char*) prompt);
			pthread_mutex_unlock(&mutex);
		}
		
	}
	pthread_exit(0);
	//close(socket);

	return (void *) 0;
}

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

void read_statefile(char *statefile_tmp) {
	//FILE * preload = statefile;
	//FILE * before = statefile;
	int i = 0, k =0;
	char end[1024];
	char line[1024];
	char entry_message[1024];

	FILE *statefile_r = fopen(statefile_tmp, "r");

	printf("entr cap = %d\n", entryCAP);
	while (fgets(line, sizeof(line), statefile_r) != NULL) {
		if (strncmp(line, "!", 1)== 0) {
			//printf("%s [%d]\n", line, i);
			i++;
		}
	}
	memset(line, 0, sizeof(line));
	memset(end, 0, sizeof(end));
	memset(entry_message, 0, sizeof(entry_message));

	entryCAP = i;
	entry = (char **) malloc (entryCAP*sizeof(char*));

	
	//printf("this is i: %d\n", i);
	i = 0;
	rewind(statefile_r);
	while (fgets(line, sizeof(line), statefile_r) != NULL) {
		//printf("checkpoint0\n");
		if (strncmp(line, "!", 1)== 0) {
			entry[i] = (char *) malloc (1024*sizeof(char));
			//printf("checkpoint0a\n");
			//printf("cut %s\n", line);
			strcat(end, line);
			//printf("checkpoint0b\n");
			fgets(entry_message, sizeof(entry_message), statefile_r);
			//printf("checkpoint0c\n");
			//printf("end %s\n", end);
			strcat(end, entry_message);
			//printf("checkpoint0d\n");
			//printf("checkpoint\n");
			//printf("end: %s", end);
			strcpy(entry[i], end);
			//printf("checkpoint0e\n");
			//printf("entry %s end is %s",entry[i], end);
			bzero(entry_message, 1024);
			bzero(end, 1024);
			bzero(line, 1024);
			i++;
			//printf("checkpoint2\n");
		}
		//printf("checkpoint3\n");
	}
	//printf("Entry  is: %s \n", entry[0]);
	//printf("Entry  is: %s \n", entry[1]);
	fclose(statefile_r);
}

void free_board () {
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

void save_wb()
{
    int numMessages = entryCAP;
    statefile = fopen("statefile.txt", "w+");
    if(!statefile){
    	printf("cannot open whiteboard save file");
    }

    int p = 0, temp = 0;
    for(p; p <= numMessages-1; ++p)
    {   
        // message
        fprintf(statefile, "!%dp%lu\n%s\n",p, strlen(entry[p]), entry[p]);
       //fprintf(statefile,"!%s", entry[p]);
        printf("%s",entry[p]);
        
    }
    fflush(statefile);
    fclose(statefile);
}
void sighandle_sigterm(int sig) {
    printf(" The signal number recieved is %d \n", sig);
    //need to save the state of the wb to a statefile
    save_wb();
    free_board();
    close (snew);
    exit(1);
}
void deamonize (FILE * deamonfile) {

    int i = 0;
    pid_t pid = 0;
    pid_t sid = 0;
    FILE *fp = deamonfile;

    pid = fork();

    if (pid < 0)
    {
        printf("fork failed!\n");
        exit(1);
    }

    if (pid > 0)
    {
        // in the parent
       printf("pid of child process %d \n", pid);
       exit(0);
    }

    umask(0);

    // open a log file
    fp = fopen ("deamonfile.log", "w+");
    if(!fp){
        printf("cannot open log file");
    }

    // create new process group -- don't want to look like an orphan
    sid = setsid();
    if(sid < 0)
    {
        fprintf(fp, "cannot create new process group");
        exit(1);
    }

    /* Change the current working directory */
    // if ((chdir("/")) < 0) {
    //   printf("Could not change working directory to /\n");
    //   exit(1);
    // }

    // close standard fds
    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);
    //printf("HIHIHIHI\n");

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