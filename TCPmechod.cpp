/* TCPmechod.c - main, echo */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>

#include "typedef.hpp"
#define	QLEN		   5	/* maximum connection queue length	*/
#define	BUFSIZE		4096

extern int	errno;
int		errexit(const char *format, ...);
int		passiveTCP(const char *service, int qlen);
int		echo(Pipeline& fd, int sock);
vector<Pipeline> env(30);
Broadcast user_pool;
fd_set	rfds;			    /* read file descriptor set	    */
fd_set	afds;			    /* active file descriptor set	*/
/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ECHO service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	char	*service = "8886";	/* service name or port number	*/
	struct sockaddr_in fsin;	/* the from address of a client	*/
	int	msock;			        /* master server socket		    */
	socklen_t	alen;			        /* from-address length		    */
	int	fd, nfds;
	unordered_map<int, int> fd_user;

    signal(SIGCHLD, [](int signo) {
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0);
    });

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: TCPmechod [port]\n");
	}

	msock = passiveTCP(service, QLEN);

	nfds = 31;
	FD_ZERO(&afds);
	FD_SET(msock, &afds);

	while (1) {
		memcpy(&rfds, &afds, sizeof(rfds));
		while (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,
				(struct timeval *)0) < 0)
				{
					if (errno == 4)
						continue;
					errexit("select: %s\n", strerror(errno));
				}
			
		if (FD_ISSET(msock, &rfds)) {
			int	ssock;

			alen = sizeof(fsin);
			ssock = accept(msock, (struct sockaddr *)&fsin,&alen);
			if (ssock < 0)
				errexit("accept: %s\n",
					strerror(errno));
			fd_user[ssock] = user_pool.add_user(fsin, ssock);
			user_pool.welcome(ssock);
			user_pool.login(fd_user[ssock]);
			write(ssock, "% ", 3);
			dup2(ssock, env[fd_user[ssock]].get_pipe(0).get_in());
			FD_SET(ssock, &afds);
		}
		for (fd=0; fd<nfds; ++fd)
			if (fd != msock && FD_ISSET(fd, &rfds))
			{
				/* In the child process: */
				if (echo(env[fd_user[fd]], fd) == 0) {
					user_pool.logout(fd_user[fd]);
					user_pool.delete_user(fd_user[fd]);
					fd_user.erase(fd);
					close(fd);
					FD_CLR(fd, &afds);
				}
			}
	}
}

/*------------------------------------------------------------------------
 * echo - echo one buffer of data, returning byte count
 *------------------------------------------------------------------------
 */
int
echo(Pipeline& all, int sock)
{
	std::cout << "now sock is: " << sock << endl;
	dup2(sock, STDERR_FILENO);
	string 	input;
	char in_char[15001];
	memset(&in_char, 0, sizeof in_char);
	if (!read(sock, in_char, sizeof in_char)){
        for (int i=0; i< 2048; i++)
        {
            for (auto &j: all.get_child_proc(i))
                kill(j, SIGKILL);
        }
        ::exit(0);
        return 0;
    }
	input = in_char;
    input.pop_back();
    input.pop_back();
	if(input.empty()) {
		write(sock, "% ", 3);
		return 1;
	}
    Command cmd(input);
	int first = true;
    if (all.get_pipe(0).mode_on())
    {
        first = false;
    }
    for (int i=0; i< cmd.get_block().size(); i++)
    {
		int status;
        while ( (status = cmd.get_block()[i].execute_new(user_pool, all, first\
            , (i == cmd.get_block().size() - 1)? true: false, sock)) == 1)  // fork error
            usleep(1500);
        first = false;
        if (status != 0)
            cerr << "Fail execution on" << cmd.get_block()[i].get_argv()[0] <<endl;
    }
    all.close(0);

    auto last = cmd.get_block().back();
    if (last.get_flag() == 1)
    {
		FD_SET(sock, &rfds);
        all.add_all_proc(last.get_cnt(), all.get_child_proc(0));
    }
    for (auto &i: all.get_child_proc(0))
    {
        int status;
		cout << "waiting for " << i << endl;
        waitpid(i, &status, 0);
    }
	if (input == "exit")
		return 0;
	write(sock, "% ", 3);
	all.get_pipe(0).close();
	all.next_(); 
	cout << "finished\n";
	return 1;
}
