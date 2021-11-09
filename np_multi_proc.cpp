/* TCPdaytimed.c - main */
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include<dirent.h>  
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
#include <pthread.h>

#include "typedef.hpp"

extern int	errno;
int		errexit(const char *format, ...);
int		passiveTCP(const char *service, int qlen);
int     sem_open(key_t key);
void    sem_wait(int id);
void    sem_signal(int id);

BrstShrd user_pool;
unordered_map<int, int> fd_user;
pthread_t   thread_[30];

#define QLEN	31

void *shell_fifo(void *sockfd);
void *receive(void *arg);
void *getout(void *arg);
/*------------------------------------------------------------------------
 * main - Iterative TCP server for DAYTIME service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	struct	    sockaddr_in fsin;	/* the from address of a client	*/
	char	    *service = "8886";	/* service name or port number	*/
	int	        msock, ssock;		/* master & slave sockets	*/
	socklen_t	alen;		        /* from-address length		*/
	

    signal(SIGUSR2, [](int signo) {
        cout << "triggered\n";
        pthread_join(thread_[user_pool.cur], NULL);
        cout << "finished\n";
    });

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
	DIR *mydir = NULL;
	if ( (mydir = opendir("./user_pipe")) == NULL) {
    		cout << " construct directory\n";
        int ret = chmod(".", 0755);
		if (ret != 0)
		{
			cout <<errno << endl;
			errexit("cannot change permission\n");
		}

        ret = mkdir("./user_pipe", 0700);
		if (ret != 0)
		{
			cout <<errno << endl;
			errexit("cannot create directory\n");
		}
		cout << "finish\n";	
	}
	else
		cout << "existed directory\n";

	while (1) {
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen);

		fd_user[ssock] = user_pool.add_user(fsin, ssock);

		if (ssock < 0)
			errexit("accept failed: %s\n", strerror(errno));
        pthread_create(&thread_[fd_user[ssock]], NULL, shell_fifo, &ssock);
	}
}

void *shell_fifo(void *sockfd){
    // change the fd of stdout to socket
    int fd = *(int *)sockfd;
    int clisem;
    key_t sem_key2 = 8891;
    if ( (clisem = sem_open(sem_key2)) < 0) 
            errexit("...");

    sem_wait(clisem);	
    user_pool.welcome(fd);
    user_pool.login(fd);
    sem_signal(clisem);	

    Pipeline all;  
    dup2(fd, all.get_pipe(0).get_in());
    string input;
    // set the signal handler
    signal(SIGCHLD, [](int signo) {
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0);
    });

    signal(SIGUSR1, [](int signo) {
        pthread_t temp, temp2;
        pthread_create(&temp, NULL, receive, NULL);
        pthread_create(&temp2, NULL, getout, NULL);
        pthread_join(temp, NULL);
        pthread_join(temp2, NULL);
    });

    while(1){
		write(fd, "% ", strlen("% "));
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
        while (input.back() == '\r' || input.back() == '\n')
        {
            input.pop_back();
        }
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
        if (input == "exit")
        {
            fd_user.erase(fd);
		    return NULL;
        }
	    all.next_(); 
    }

    return 0;
}

void *receive(void *arg)
{
    int readfd;
    if ( (readfd = open(user_pool.pipes.back().c_str(), O_RDONLY, 0666)) < 0)
		errexit("server: can't open read fifo: %s", user_pool.pipes.back().c_str());
    user_pool.out_fd.push_back(readfd);
}

void *getout(void *arg)
{
    int writefd;
    if ( (writefd = open(user_pool.pipes.back().c_str(), O_WRONLY, 0666)) < 0)
		errexit("server: can't open write fifo: %s", user_pool.pipes.back().c_str());
    user_pool.in_fd.push_back(writefd);
}
