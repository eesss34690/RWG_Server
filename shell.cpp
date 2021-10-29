#include <string.h>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "typedef.hpp"

using namespace std;

int shell(int fd){
    // change the fd of stdout to socket
    int fd_temp[2];
    ::pipe(fd_temp);
    ::dup2(fd_temp[1], STDOUT_FILENO);
    close fd_temp[0];

    char *percent = "% ";
    char buf[15001];
    int cc;

    setenv("PATH", "bin:.", 1);
    Pipeline all;  
    // set the signal handler
    signal(SIGCHLD, [](int signo) {
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0);
    }); 
    
    write(fd, percent, sizeof(percent));	
    while(cc = read(fd, buf, sizeof(buf))){
       if (cc < 0) {
		for (int i=0; i< 2048; i++)
		{
			for (auto &j: all.get_child_proc(i))
				kill(j, SIGKILL);
		}
		::exit(0);
		return 0;
        } 	
	string input(buf); 
        if(input.empty()) continue;
            Command cmd(input);

        // get the fd table now
        //Pipe_IO shell_fd = all.get_pipe(0);
        //if (!shell_fd.mode_on())
        //	shell_fd.construct_pipe();
        
        int first = true;
        if (all.get_pipe(0).mode_on())
        {
            first = false;
        }
        for (int i=0; i< cmd.get_block().size(); i++)
        {
            int status;
            while ( (status = cmd.get_block()[i].execute(all, first\
                , (i == cmd.get_block().size() - 1)? true: false)) == 1, \
		fd_temp[1])  // fork error
                usleep(1500);
            first = false;
            if (status != 0)
                cerr << "Fail execution on" << cmd.get_block()[i].get_argv()[0] <<endl;
            usleep(1500);
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
        write(fd, percent, sizeof(percent));	
    

    }

    return 0;
}
