#include <sys/types.h>
#include <vector>
#include "typedef.hpp"

void Pipeline::add_pipe(int offset)
{
	int num = get_num(offset);
	cout << "add_pipe: " << num <<endl;
	cout <<"mode: " <<m_pipes[num].mode_on() <<endl;
	if (m_pipes[num].mode_on() == false)
		m_pipes[num] = Pipe_IO::create();
}

void Pipeline::close_all()
{
	for (int i = 0; i< MaxForks; i++)
	{
		m_pipes[i].close();
	}
}

void Pipeline::add_process(int offset, pid_t child_pid)
{
	int num = get_num(offset);
	m_child_proc[num].push_back(child_pid);
}

void Pipeline::add_all_proc(int offset, vector<int> child_pid)
{
	int num = get_num(offset);
	m_child_proc[num].insert(m_child_proc[num].end(), child_pid.begin(), child_pid.end());
	child_pid.clear();
}

void Pipeline::next_()
{
	m_pipes[now].close();
	m_child_proc[now].clear();
	now = (now == 2047)? 0: (now + 1);
}
