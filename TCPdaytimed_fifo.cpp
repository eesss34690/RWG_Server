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
#include <vector>
#include <unordered_map>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>

#include "typedef.hpp"

extern int	errno;
int		errexit(const char *format, ...);
int		passiveTCP(const char *service, int qlen);
vector<Pipeline> env(30);
BrstShrd user_pool;

#define QLEN	5

int shell_fifo(int fd);
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
	unordered_map<int, int> fd_user;

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
		fd_user[ssock] = user_pool.add_user(fsin, ssock);
		//user_pool.welcome(ssock);
		//user_pool.login(fd_user[ssock]);
		dup2(ssock, env[fd_user[ssock]].get_pipe(0).get_in());
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
			//dup2( ssock, STDIN_FILENO );
			//dup2( ssock, STDOUT_FILENO );  /* duplicate socket on stdout */
			dup2( ssock, STDERR_FILENO );  /* duplicate socket on stderr too */
			shell_fifo(ssock);
			close(ssock);
		}
	}
}


int shell_fifo(int fd){
    // change the fd of stdout to socket
 
    //setenv("PATH", "bin:.", 1);
    Pipeline all;  
    string input;
    // set the signal handler
    signal(SIGCHLD, [](int signo) {
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0);
    });
    while(1){
		write(fd, "% ", 3);
        char in_char[15001];
		memset(&in_char, 0, sizeof in_char);
		if (!read(fd, in_char, sizeof in_char)){
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
        if(input.empty()) continue;
        Command cmd(input);

        int first = true;
        if (all.get_pipe(0).mode_on())
        {
            first = false;
        }
        for (int i=0; i< cmd.get_block().size(); i++)
        {
            int status;
            while ( (status = cmd.get_block()[i].execute_fifo(user_pool, all, first\
            , (i == cmd.get_block().size() - 1)? true: false, fd)) == 1)  // fork error
                usleep(1500);
            first = false;
            if (status != 0)
                cerr << "Fail execution on" << cmd.get_block()[i].get_argv()[0] <<endl;
        }
        all.close(0);

        auto last = cmd.get_block().back();
        if (last.get_flag() == 1)
        {
            all.add_all_proc(last.get_cnt(), all.get_child_proc(0));
        }
        for (auto &i: all.get_child_proc(0))
        {
            int status;
            waitpid(i, &status, 0);
        }
	all.next_(); 
    

    }

    return 0;
}
