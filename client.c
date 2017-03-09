#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define	 MY_PORT  2229

/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.
 --------------------------------------------------------------------- */

int main()
{
	int	s, number;
	char msg[100];
	char * Fupdate = "?12p\n";
	//update:"@12p4\ncool\n";

	struct	sockaddr_in	server;
	struct	hostent		*host;

	host = gethostbyname ("localhost");

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}

	//while (1) {
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
		
		printf("sending: %s \n", Fupdate);
		bzero(msg, 100);
		recv(s, msg, 100, 0);
		printf("%s\n", msg);
		sleep (2);

		send(s,Fupdate, 100, 0);
		sleep(2);

		bzero(msg, 100);
		recv(s, msg, 100, 0);
		printf("%s\n", msg);
		sleep (2);
		close (s);
		//fprintf (stderr, "Process %d gets number %d\n", getpid (), ntohl (number));
	//}
}
