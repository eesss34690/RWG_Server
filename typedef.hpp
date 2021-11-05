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
#include <netinet/in.h>
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
	//int now;
	int get_num(int offset) {return (now + offset) % MaxForks;}
public:
	int now;
	Pipeline() {now = 0; m_exit_child.resize(0); setenv("PATH", "bin:.", 1);}
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

class Broadcast
{
public:
	vector<string> users;
	int smallest;
	vector<string> ip;
	vector<string> ports;
	vector<int> socket;
	char    sbuf[15000];
public:
	vector<int> in_fd;
	vector<int> out_fd;
	vector<Pipe_IO> pipes;
	Broadcast();
	int add_user(sockaddr_in fsin, int sock);
	void update_small();
	void change_name(string name, int idx){users[idx] = name;}
	void welcome(int fd);
	void login(int id);
	void logout(int id);
	void brst_msg();
	void delete_user(int id);
	void who(int fd);
	void name(string new_, int fd);
	void tell(string msg, int fd, int to_id);
	void yell(string msg, int fd);
	int get_in(string cmd, int fd);
	int get_out(int fd, string cmd);
};

#define	MAXMESGDATA	(4096-16)
#define	MESGHDRSIZE	(sizeof(Mesg) - MAXMESGDATA)
typedef struct {
  int	mesg_len;	/* #bytes in mesg_data, can be 0 or > 0 */
  long	mesg_type;	/* message type, must be > 0 */
  char	mesg_data[MAXMESGDATA];
} Mesg;
class BrstShrd
{
public:
	key_t			shr_key1;		// shared memory
	Mesg	*		users;			// ptr to message structure
	key_t			shr_key2;		// shared memory
	Mesg	*		ip;				// ptr to message structure
	key_t			shr_key3;		// shared memory
	Mesg	*		port;			// ptr to message structure
	key_t			shr_key4;		// shared memory
	Mesg	*		socket;			// ptr to message structure
	key_t			shr_key5;		// shared memory
	Mesg	*		in_fd;			// ptr to message structure
	key_t			shr_key6;		// shared memory
	Mesg	*		out_fd;			// ptr to message structure
	key_t			shr_key7;		// shared memory
	Mesg	*		pipes;			// ptr to message structure
	key_t			sem_key1;		// shared memory
	key_t			sem_key2;		// shared memory
	int 			clisem, servsem;
public:
	BrstShrd();
	int add_user(sockaddr_in fsin, int sock);
	/*
	void change_name(string name, int idx){users[idx] = name;}
	void welcome(int fd);
	void login(int id);
	void logout(int id);
	void brst_msg();
	void delete_user(int id);
	void who(int fd);
	void name(string new_, int fd);
	void tell(string msg, int fd, int to_id);
	void yell(string msg, int fd);
	string get_in(string cmd, int fd);
	string get_out(int fd, string cmd);
	*/
};

class Pipe_block
{
private:
	Pipe_IO 	m_pipe;
	int 		m_num;
	int 		m_flag;
	int 		m_in;
	string 		m_filename;
	string 		spec_pipe;
	vector<string>	m_argv;
	int printenv( int fd);
	int setenv();
	char ** parse_arg();
public:
	Pipe_block();
	int execute(Pipeline& all, bool first, bool last);
	int execute_new(Broadcast& env, Pipeline& all, bool first, bool last, int sock);
	int execute_fifo(BrstShrd& env, Pipeline& all, bool first, bool last, int sock);
	void set_cnt(int num) {m_num = num;}
	void set_flag(int flag) {m_flag = flag;}
	void set_in(int in) {m_in = in;}
	void set_file(string filename) {m_filename = filename;}
	void set_spec(string cmd) {spec_pipe = cmd;}
	void set_argv(vector<string> argv) {m_argv = argv;}
	int get_cnt() {return m_num;}
	int get_flag() {return m_flag;}
	int get_in() {return m_in;}
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
