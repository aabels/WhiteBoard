#ifndef WBS379_H_   /* Include guard */
#define WBS379_H_

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
int entryCAP = 0, state_flag = 0;
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
	p = command;
	action[0] = 0;
	action[1] = 0;
    printf("Handle[0]: %s Handle[1] %s\n", handle[0], handle[1]);
    
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
					strncpy(type, &(*p), 1);
					flag = 0;
				}
			}
			p++;
		}

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

    	memcpy((void *)query, (const void *)entry[index], sizeof(char)*1024);

    	//Error handling for when the entry is empty
    	if (strlen(entry[index]) == 0) {
    		bzero(error_msg, 1024);
    		sprintf(error_msg+strlen(error_msg), "!%lup0\n",(action[0]+1));
    		send(connection, error_msg, 40, 0);
    		memset(error_msg, 0, 1024);
    		return (void *) 0;
    	}
		
    	//to accomodate the entry numbers starting at 1
    	//locate query here with wb[index] and send results to client
    	//using the size of result bc no extra allocation implimented yet
    	if (state_flag == 0) {
    		sprintf(result+strlen(result), "!%ld%s%ld\n", (action[0]+1), type, action[1]);
    		sprintf(result+strlen(result), "%s", query);
    	}
    	else {
    		sprintf(result+strlen(result), "%s", query);
    	}
    	send(connection, result, 1024, 0);
    	memset(result, 0, sizeof(result));
    	memset(query, 0, sizeof(query));
    }
    else if (strncmp(command, "@", 1)==0) {
  		printf("DO I GO HERE\n");
		printf("action[0]: %ld action[1]: %ld\n", action[0], action[1]);

		//HANDLES CLEARING HERE
    	if ((action[1]==0)) {
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
				bzero(error_msg, 1024);
				det[index] = "p";
				memset(entry[index], '\0', sizeof(char));
				printf("entry is %s and det[%d] %s\n", entry[index], index, det[index]);
				sprintf(error_msg+strlen(error_msg), "%s%lue0", suc_msg, (action[0]+1));
				send(connection, error_msg, 40, 0);
				return (void *) 0;
				}

    	}
    	if (action[0]==0){
			bzero(error_msg, 0);
			memset(error_msg, 0, 1024);
			sprintf(error_msg+strlen(error_msg), "!%lue34\n%s", action[0], ranng_msg);
			send(connection, error_msg, 40, 0);
			memset(error_msg, 0, 1024);
	    	return (void *) 0;
		}
    	//need to check if its empty
    	action[0] = action[0] - 1;
    	
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
    	memcpy((void *)query, (const void *)entry[index], sizeof(char)*action[1]);
    	if (strlen(query)== 0) {
    		//store the new entry immediatly and keep track of the type[index]
    		memset(entry[index], 0, sizeof(char)*action[1]);
    		memset(error_msg, 0, 1024);
    		memcpy((void *)entry[index], (const void *)handle[1], sizeof(char)*action[1]);
    		sprintf(error_msg+strlen(error_msg), "%s%lue0", suc_msg, (action[0]+1));
    		send(connection, error_msg, 100, 0);
    		bzero(error_msg, 0);
    		memset(error_msg, 0, 1024);
    	}
    	else {
    		memset(entry[index], 0, sizeof(char)*action[1]);
    		memset(error_msg, 0, 1024);
    		memcpy((void *)entry[index], (const void *)handle[1], sizeof(char)*action[1]);
    		printf(" Index %d Entry %d is: %s\n", index, index, entry[index]);
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
		statefile = fopen("statefile.all", "w+");
		for (k=0; k <= entryCAP -1; k++) {
			det[k] = "p";
			fprintf(statefile, "!%dp%lu\n%s\n",k, strlen(entry[k]), entry[k]);
		}
		fclose(statefile);
	}
}

void read_statefile(char *statefile_tmp) {

	int i = 0, k =0;
	char end[1024];
	char line[1024];
	char entry_message[1024];

	FILE *statefile_r = fopen(statefile_tmp, "r");
	//To get the count of the number of entries from a statefile
	while (fgets(line, sizeof(line), statefile_r) != NULL) {
		if (strncmp(line, "!", 1)== 0) {
			i++;
		}
	}
	memset(line, 0, sizeof(line));
	memset(end, 0, sizeof(end));
	memset(entry_message, 0, sizeof(entry_message));

	entryCAP = i;
	entry = (char **) malloc (entryCAP*sizeof(char*));
	i = 0;
	rewind(statefile_r);
	while (fgets(line, sizeof(line), statefile_r) != NULL) {
		if (strncmp(line, "!", 1)== 0) {
			entry[i] = (char *) malloc (1024*sizeof(char));
			strcat(end, line);
			fgets(entry_message, sizeof(entry_message), statefile_r);
			strcat(end, entry_message);
			strcpy(entry[i], end);
			bzero(entry_message, 1024);
			bzero(end, 1024);
			bzero(line, 1024);
			i++;
		}
	}
	fclose(statefile_r);
}
//Free the malloced WB
void free_board () {
	int j;
	for (j=0; j <= entryCAP -1; j++) {
		free(entry[j]);
	}
	free(entry);
}

//Save the whiteboard status to statefile
void save_wb()
{
    int numMessages = entryCAP;
    statefile = fopen("statefile.all", "w+");
    if(!statefile){
    	printf("cannot open whiteboard save file");
    }

    int p = 0, temp = 0;
    for(p; p <= numMessages-1; ++p)
    {   
        // message
        fprintf(statefile, "!%d%s%lu\n%s\n",p, det[p], strlen(entry[p]), entry[p]);    
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

}

#endif // WBS379_H_