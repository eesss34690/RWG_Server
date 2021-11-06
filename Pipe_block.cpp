#include <stdlib.h>
#include <iostream>
#include "typedef.hpp"
#define BUFFERSIZE 4096 
#define MaxForks   2048
using namespace std;

int		errexit(const char *format, ...);

Pipe_block::Pipe_block()
{
	m_num = 0;
	m_flag = 0;
	m_in = 0;
}

int Pipe_block::setenv_sin(Broadcast& env, int sock)
{
	if (m_argv.size() < 3)
		cerr << "Invalid arguments: setenv\n";
	else
	{
		::setenv(m_argv[1].c_str(), m_argv[2].c_str(), 1);
		env.update_env(m_argv[1], m_argv[2], sock);
	}
	return 0;
}

int Pipe_block::printenv(int fd = 1)
{
	if (m_argv.size() < 2)
		cerr << "Invalid arguments: printenv\n";
	else
	{
		const char * env = getenv(m_argv[1].c_str());
		if (env != NULL)
		{
			//cout << env <<endl;
			if (fd != 1)
			{
				write(fd, env, strlen(env));
				write(fd, "\n", strlen("\n"));
			}
			else
				cout << env << endl;

		}
	}
	return 0;
}

int Pipe_block::setenv()
{
	if (m_argv.size() < 3)
		cerr << "Invalid arguments: setenv\n";
	else
		::setenv(m_argv[1].c_str(), m_argv[2].c_str(), 1);
	return 0;
}

char ** Pipe_block::parse_arg()
{
	char** arg = new char*[m_argv.size() + 1];
	size_t i = 0;
	for (; i< m_argv.size(); i++)
	{
		//cout << "arg: "<< m_argv.at(i) << endl;
		arg[i] = strdup(m_argv.at(i).c_str());
	}
	arg[i] = NULL;
	return arg;
}

int Pipe_block::execute(Pipeline& all, bool first, bool last)
{	
	if (m_flag == 3)
	{
		if (m_argv[0] == "printenv")
			return printenv();
		else if (m_argv[0] == "setenv")
			return setenv();
	}	
	else if (m_flag == 5)
	{

		for (int i=0; i< MaxForks; i++)
		{
			for (auto &j: all.get_child_proc(i))
			{
				kill(j, SIGKILL);
			}
		}
		exit(0);	
		return 0;
	}
	{
		m_pipe = all.get_pipe(0);
		Pipe_IO new_fd;
		if (m_flag < 2&& all.get_pipe(m_num).mode_on())
		{
			new_fd = all.get_pipe(m_num);
		}
		else
			new_fd = Pipe_IO::create();
		
		// fork
		pid_t child_pid = fork();
		if (child_pid < 0)
			return 1;
		// parent proc.
		else if (child_pid > 0)
		{
			// child will do the job, close it
			m_pipe.close();
			all.set_pipe(m_num, new_fd);
			all.add_process(m_num, child_pid);
			return 0;
		}
		// child proc.
		else 
		{
			// deal with fd table duplication
			// case 1: !N (0)
			if (m_pipe.mode_on())
				dup2(m_pipe.get_in(), STDIN_FILENO);
			if (m_flag == 0)
			{
				if (!first)
				{
					int fd = m_pipe.get_in();
					if (fd != -1)
					{
						dup2(fd, STDIN_FILENO);
					}
				}
				auto fd = new_fd.get_out();
				if (fd != -1)
					dup2(fd, STDERR_FILENO);
				if (fd != -1)
					dup2(fd, STDOUT_FILENO);
			}
			// case 2: |N (1)
			else if (m_flag == 1)
			{
				if (!first)
				{
					int fd = m_pipe.get_in();
					if (fd != -1)
					{
						dup2(fd, STDIN_FILENO);
					}
				}
				auto fd = new_fd.get_out();
				if (fd != -1)
					if (dup2(fd, STDOUT_FILENO) < 0)
						errexit("dup2: %s\n", strerror(errno));
			}
			else if (m_flag == 2)
			{
				int fd_file = open(m_filename.c_str(), (O_RDWR | O_CREAT | O_TRUNC), 0644);
				dup2(fd_file, STDOUT_FILENO);
				if (!first)
				{
					int fd = m_pipe.get_in();
					if (fd != -1)
					{
						dup2(fd, STDIN_FILENO);
					}
				}
			}
			else if (m_flag > 2)
			{
				if (!first)
				{
					int fd = m_pipe.get_in();
					if (fd != -1)
					{
						dup2(fd, STDIN_FILENO);
					}
				}
				if (!last)
				{
					int fd = new_fd.get_out();
					if (fd != -1)
					{
						dup2(fd, STDOUT_FILENO);	
					}
				}	
			}
			// finish fd table reassignment, close it
			m_pipe.close();
			new_fd.close();
			//all.close_all();
			// execution
			
			char ** arg = parse_arg();
	
			if (m_flag == 3)
			{
				if (m_argv[0] == "printenv")
					return printenv();
				else if (m_argv[0] == "setenv")
					return setenv();
			}
			else if (execvp(m_argv[0].c_str(), arg) < 0)
			{
				cerr << "Unknown command: [" << m_argv[0] << "]." << endl;
				exit(0);
			}
	
			return 0;
		}
	}
	return 0;
}

int Pipe_block::execute_new(Broadcast& env, Pipeline& all, bool first, bool last, int sock)
{	
	cout << "my flag: "<< m_flag << endl;
	if (m_flag == 3)
	{
		if (m_argv[0] == "printenv")
			return printenv(sock);
		else if (m_argv[0] == "setenv")
			return setenv_sin(env, sock);
	}	
	else if (m_flag == 5)
	{
		all.close_all();
		for (int i=0; i< MaxForks; i++)
		{
			for (auto &j: all.get_child_proc(i))
			{
				kill(j, SIGKILL);
			}
		}
		//exit(0);	
		return 0;
	}
	else if (m_flag == -1)
	{
		env.who(sock);
	}	
	else if (m_flag == -2)
	{
		env.name(m_argv[0], sock);
	}	
	else if (m_flag == -3)
	{
		env.tell(m_argv[1], sock, std::stoi(m_argv[0]));
	}	
	else if (m_flag == -4)
	{
		env.yell(m_argv[0], sock);
	}	
	else
	{
		int fd_5, fd_in;	
		if (m_in)
		{
			fd_in = env.get_in(spec_pipe, sock);
			if (fd_in == -1)
				return 0;
		}
		if (m_flag == -5)
		{
			fd_5 = env.get_out(sock, spec_pipe);
		}
	
		
		m_pipe = all.get_pipe(0);
		cout << "current out: "<< m_pipe.get_out() << endl;
		Pipe_IO new_fd;
		if (m_flag> -1&& m_flag < 2&& all.get_pipe(m_num).mode_on())
		{
			new_fd = all.get_pipe(m_num);
		}
		else
			new_fd = Pipe_IO::create();
		
		// fork
		pid_t child_pid = fork();
		if (child_pid < 0)
			return 1;
		// parent proc.
		else if (child_pid > 0)
		{
			// child will do the job, close it
			if (m_flag == -5)
			{
				::close(fd_5);
				fd_5 = -1;
			}
			if (m_in)
			{
				::close(fd_in);
				fd_in = -1;
			}
			m_pipe.close();
			all.set_pipe(m_num, new_fd);
			all.add_process(m_num, child_pid);
			return 0;
		}
		// child proc.
		else 
		{
			// deal with in table
			if (!first)
			{
				int fd = m_pipe.get_in();
				if (fd != -1)
				{
					dup2(fd, STDIN_FILENO);
				}
			}
			else if (m_in)
			{
				//if (fd_in != -1)
				//{
					dup2(fd_in, STDIN_FILENO);
					::close(fd_in);
					fd_in = -1;
				//}
			}
			
			//deal with out table
			// case 1: !N (0)
			if (m_flag == 0)
			{
				auto fd = new_fd.get_out();
				if (fd != -1)
					dup2(fd, STDERR_FILENO);
				if (fd != -1)
					dup2(fd, STDOUT_FILENO);
			}
			// case 2: |N (1)
			else if (m_flag == 1)
			{
				auto fd = new_fd.get_out();
				if (fd != -1)
				{
					dup2(fd, STDOUT_FILENO);
				}
				dup2(sock, STDERR_FILENO);
			}
			else if (m_flag == 2)
			{
				int fd_file = open(m_filename.c_str(), (O_RDWR | O_CREAT | O_TRUNC), 0644);
				dup2(fd_file, STDOUT_FILENO);
				dup2(sock, STDERR_FILENO);
			}
			else if (m_flag == -5)
			{
				if (fd_5 != -1)
				{
					dup2(fd_5, STDOUT_FILENO);	
					::close(fd_5);
					fd_5 = -1;
				}
				else
				{
					std::cout.rdbuf( NULL );
				}
				dup2(sock, STDERR_FILENO);
			}
			else if (m_flag > 2)
			{
				if (!last)
				{
					int fd = new_fd.get_out();
					if (fd != -1)
					{
						dup2(fd, STDOUT_FILENO);
						dup2(sock, STDERR_FILENO);	
					}
				}
				else
				{
					dup2(sock, STDOUT_FILENO);
					dup2(sock, STDERR_FILENO);
				}	
			}
			
			// finish fd table reassignment, close it
			m_pipe.close();
			new_fd.close();
			// execution
			
			char ** arg = parse_arg();
	
			if (m_flag == 3)
			{
				if (m_argv[0] == "printenv")
					return printenv();
				else if (m_argv[0] == "setenv")
					return setenv();
			}
			else if (execvp(m_argv[0].c_str(), arg) < 0)
			{
					cerr << "Unknown command: [" << m_argv[0] << "]." << endl;
					exit(0);
			}
	
			return 0;
		}
	}
	return 0;
}

int Pipe_block::execute_fifo(BrstShrd& env, Pipeline& all, bool first, bool last, int sock)
{	
	cout << "my flag: "<< m_flag << endl;
	if (m_flag == 3)
	{
		if (m_argv[0] == "printenv")
			return printenv();
		else if (m_argv[0] == "setenv")
			return setenv();
	}	
	else if (m_flag == 5)
	{
		all.close_all();
		for (int i=0; i< MaxForks; i++)
		{
			for (auto &j: all.get_child_proc(i))
			{
				kill(j, SIGKILL);
			}
		}
		//exit(0);	
		return 0;
	}
	/*
	else if (m_flag == -1)
	{
		env.who(sock);
	}	
	else if (m_flag == -2)
	{
		env.name(m_argv[0], sock);
	}	
	else if (m_flag == -3)
	{
		env.tell(m_argv[1], sock, std::stoi(m_argv[0]));
	}	
	else if (m_flag == -4)
	{
		env.yell(m_argv[0], sock);
	}
	*/	
	else
	{
		string fd_5, fd_in;
		int readfd, writefd;
		/*
		if (m_flag == -5)
		{
			fd_5 = env.get_out(sock, spec_pipe);
			if ( (writefd = open(fd_5.c_str(), 1)) < 0)
 				printf("server: can't open write fifo: %s", fd_5.c_str()); 
		}
		
		if (m_in)
		{
			fd_in = env.get_in(spec_pipe, sock);
			if ( (readfd = open(fd_in.c_str(), 0)) < 0)
 				printf("server: can't open read fifo: %s", fd_in.c_str()); 
		}
		*/
		m_pipe = all.get_pipe(0);
		cout << "current out: "<< m_pipe.get_out() << endl;
		Pipe_IO new_fd;
		if (m_flag> -1&& m_flag < 2&& all.get_pipe(m_num).mode_on())
		{
			new_fd = all.get_pipe(m_num);
		}
		else
			new_fd = Pipe_IO::create();
		
		// fork
		pid_t child_pid = fork();
		if (child_pid < 0)
			return 1;
		// parent proc.
		else if (child_pid > 0)
		{
			// child will do the job, close it
			if (m_flag == -5)
			{
				::close(writefd);
				writefd = -1;
			}
			if (m_in)
			{
				::close(readfd);
				readfd = -1;
			}
			m_pipe.close();
			all.set_pipe(m_num, new_fd);
			all.add_process(m_num, child_pid);
			return 0;
		}
		// child proc.
		else 
		{
			// deal with in table
			if (!first)
			{
				int fd = m_pipe.get_in();
				if (fd != -1)
				{
					dup2(fd, STDIN_FILENO);
				}
			}
			else if (m_in)
			{
				if (readfd != -1)
				{
					dup2(readfd, STDIN_FILENO);
					::close(readfd);
					readfd = -1;
				}
			}
			
			//deal with out table
			// case 1: !N (0)
			if (m_flag == 0)
			{
				auto fd = new_fd.get_out();
				if (fd != -1)
					dup2(fd, STDERR_FILENO);
				if (fd != -1)
					dup2(fd, STDOUT_FILENO);
			}
			// case 2: |N (1)
			else if (m_flag == 1)
			{
				auto fd = new_fd.get_out();
				if (fd != -1)
				{
					dup2(fd, STDOUT_FILENO);
				}
			}
			else if (m_flag == 2)
			{
				int fd_file = open(m_filename.c_str(), (O_RDWR | O_CREAT | O_TRUNC), 0644);
				dup2(fd_file, STDOUT_FILENO);
			}
			else if (m_flag == -5)
			{
				if (writefd != -1)
				{
					dup2(writefd, STDOUT_FILENO);	
					::close(writefd);
					writefd = -1;
				}
			}
			else if (m_flag > 2)
			{
				if (!last)
				{
					int fd = new_fd.get_out();
					if (fd != -1)
					{
						dup2(fd, STDOUT_FILENO);	
					}
				}
				else
				{
					dup2(sock, STDOUT_FILENO);
				}	
			}
			
			// finish fd table reassignment, close it
			m_pipe.close();
			new_fd.close();
			// execution
			
			char ** arg = parse_arg();
	
			if (m_flag == 3)
			{
				if (m_argv[0] == "printenv")
					return printenv();
				else if (m_argv[0] == "setenv")
					return setenv();
			}
			else if (execvp(m_argv[0].c_str(), arg) < 0)
			{
					cerr << "Unknown command: [" << m_argv[0] << "]." << endl;
					exit(0);
			}
	
			return 0;
		}
	}
	return 0;
}
