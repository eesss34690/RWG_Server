#include <string>
#include <vector>
#include <unistd.h>
#include <list>
#include <queue>
#include <fstream>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MaxForks    2048
using namespace std;

class Pipe_IO
{
private:
	bool exist;
	int fd_table[2];
public:
	Pipe_IO();
	static Pipe_IO create();
	void construct_pipe();
	int get_out() {return fd_table[1];}
	int get_in() {return fd_table[0];}
	bool mode_on() {return (fd_table[0] != -1 || fd_table[1] != -1);}
	int close();
		
};

class Pipeline
{
private:
	Pipe_IO m_pipes[MaxForks];
	vector<pid_t> m_child_proc[MaxForks];
	vector<pid_t> m_exit_child;
	int now;
	int get_num(int offset) {return (now + offset) % MaxForks;}
public:
	Pipeline() {now = 0; m_exit_child.resize(0);}
	Pipe_IO& get_pipe(int offset) {return m_pipes[get_num(offset)];}
	void set_pipe(int offset, Pipe_IO pipe) {m_pipes[get_num(offset)] = pipe;}
	void add_pipe(int offset);
	void close(int offset) {m_pipes[get_num(offset)].close();}
	void close_all();
	vector<pid_t> get_child_proc(int num) {return m_child_proc[get_num(num)];}
	void add_process(int offset, pid_t child_pid);
	void add_all_proc(int offset, vector<pid_t>child_pid);
	void add_exit(pid_t child) {m_exit_child.push_back(child);}
	vector<pid_t> get_exit() {return m_exit_child;}
	void next_();
};

class Pipe_block
{
private:
	bool 		exist;
	Pipe_IO         m_pipe;
	int 		m_num;
	int 		m_flag;
	string 		m_filename;
	vector<string>	m_argv;
	int printenv();
	int setenv();
	char ** parse_arg();
public:
	Pipe_block();
	int execute(Pipeline& all, bool first, bool last);
	void set_cnt(int num) {m_num = num;}
	void set_flag(int flag) {m_flag = flag;}
	void set_file(string filename) {m_filename = filename;}
	void set_argv(vector<string> argv) {m_argv = argv;}
	int get_cnt() {return m_num;}
	int get_flag() {return m_flag;}
	string get_file() {return m_filename;}
	vector<string> get_argv() {return m_argv;}
};

class Command
{
private:
	vector<Pipe_block> m_block;
	size_t find_char(string cmd, char target, int start_idx);
	string separate_output(string cmd, int start_idx, int end_idx);	

public:
	Command(string cmd);
	vector<Pipe_block> get_block () {return m_block;}

};
