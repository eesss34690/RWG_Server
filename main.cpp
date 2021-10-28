#include <string>
#include <iostream>
#include <signal.h>
#include "typedef.hpp"

using namespace std;

int main(int argc, char *argv[]){
    setenv("PATH", "bin:.", 1);
    string input;
    Pipeline all;  
    // set the signal handler
    signal(SIGCHLD, [](int signo) {
        int status;
        while (waitpid(-1, &status, WNOHANG) > 0);
    }); 
    while(1){
        cout << "% ";
        if (!getline(cin, input)) {
		for (int i=0; i< 2048; i++)
		{
			for (auto &j: all.get_child_proc(i))
				kill(j, SIGKILL);
		}
		::exit(0);
		return 0;
        } 
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
			, (i == cmd.get_block().size() - 1)? true: false)) == 1)  // fork error
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

    }

    return 0;
}
