#include "typedef.hpp"
Pipe_IO::Pipe_IO()
{
	fd_table[0] = -1;
	fd_table[1] = -1;
}

Pipe_IO Pipe_IO::create()
{
	Pipe_IO pipe;
	if (::pipe(pipe.fd_table) < 0)
		cerr << "pipe create fail\n";
	return pipe;
}

void Pipe_IO::construct_pipe()
{
	if (::pipe(fd_table) < 0)
		cerr << "pipe create fail!!!\n";
}

int Pipe_IO::close()
{
	int result = 0;
	if (fd_table[0] > -1)
	{
		::close(fd_table[0]);
		fd_table[0] = -1;
	}
	if (fd_table[1] > -1)
	{
		::close(fd_table[1]);
		fd_table[1] = -1;
	}
	return result;
}
