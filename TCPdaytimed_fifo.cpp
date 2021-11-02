/* TCPdaytimed.c - main */
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

extern int	errno;
int		errexit(const char *format, ...);
int		passiveTCP(const char *service, int qlen);

#define QLEN	5

int shell(int fd);
/*------------------------------------------------------------------------
 * main - Iterative TCP server for DAYTIME service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	struct	sockaddr_in fsin;	/* the from address of a client	*/
	char	*service = "8886";	/* service name or port number	*/
	int	msock, ssock;		/* master & slave sockets	*/
	socklen_t	alen;		/* from-address length		*/
	pid_t   cpid;

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: TCPdaytimed [port]\n");
	}

	msock = passiveTCP(service, QLEN);

	while (1) {
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
		if (ssock < 0)
			errexit("accept failed: %s\n", strerror(errno));
		cpid = fork();
		if (cpid < 0) exit(1);  /* exit if fork() fails */
		if ( cpid ) {
			/* In the parent process: */
			close( ssock ); /* csock is not needed in the parent after the fork */
			//waitpid( cpid, NULL, 0 ); /* wait for and reap child process */
		} else {
			/* In the child process: */
			dup2( ssock, STDIN_FILENO );
			dup2( ssock, STDOUT_FILENO );  /* duplicate socket on stdout */
			dup2( ssock, STDERR_FILENO );  /* duplicate socket on stderr too */
			close(ssock);
			shell(ssock);
		}
	}
}
