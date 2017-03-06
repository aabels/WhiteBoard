#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define	 MY_PORT  2224

/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.
 --------------------------------------------------------------------- */

int main()
{
	int	s, number;
	char msg[100];

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
		//if client say wants to look at the 12th entry
		//has a protocol
		//''?12\n'  have to add newline char
		//servre sends
		//@12p5\Hello\n
		//read (s, &number, sizeof (number));
		bzero(msg, 100);
		//printf("before recv\n");
		recv(s, msg, 100, 0);
		//printf("After recv\n");
		printf("%s\n", msg);
		close (s);
		//fprintf (stderr, "Process %d gets number %d\n", getpid (), ntohl (number));
		sleep (2);
	//}
}
