#include <stdlib.h>
#include <iostream>
#include <string>
#include "typedef.hpp"

#define BUFFERSIZE 4096 

using namespace std;

size_t Command::find_char(string cmd, char target, int start_idx)
{
	return cmd.find(target, start_idx);
}

string Command::separate_output(string cmd, int start_idx, int end_idx)
{
	while (cmd[start_idx] == ' ')
		start_idx++;
	while (cmd[--end_idx] == ' ');
	return cmd.substr(start_idx, 1 + end_idx - start_idx);		
}

Command::Command( string cmd )
{
	auto end_idx = 0;
	auto start_idx = 0;
	while (end_idx < cmd.length())
	{
		m_block.push_back( *(new Pipe_block) );	
		end_idx = find_char(cmd, '|', start_idx);
		if (end_idx == string::npos)
			end_idx = cmd.length();
		
		m_block.back().set_flag(4);	
		// find the command action 
		string single_cmd = separate_output(cmd, start_idx, end_idx);
		auto space = find_char(single_cmd, ' ', 0);
		string action = separate_output(single_cmd, 0, space);
		//cout << action << endl;
		if (action == "tell" )
		{
			auto space_2 = find_char(single_cmd, ' ', space + 1);
			m_block.back().set_flag(-3);
			vector<string> real = {separate_output(single_cmd, space + 1, space_2), \
				separate_output(single_cmd, space_2 + 1, single_cmd.length())};
			m_block.back().set_argv(real);
		}
		else if (action == "yell" )
		{
			m_block.back().set_flag(-4);
			vector<string> real = {separate_output(single_cmd, space + 1, single_cmd.length())};
			m_block.back().set_argv(real);
		}
		else if (action == "who" )
		{
			m_block.back().set_flag(-1);
		}
		else if (action == "name" )
		{
			m_block.back().set_flag(-2);
			vector<string> real = {separate_output(single_cmd, space + 1, single_cmd.length())};
			m_block.back().set_argv(real);
		}
		else
		{
			// special case: |N
			// the counter of redirect output should be set
			// note the cmd as redirect command
			if (end_idx < cmd.length() && cmd[1 + end_idx] != ' ')
			{
				m_block.back().set_cnt(stoi(separate_output(cmd, 1 + end_idx, cmd.length())));
				end_idx = cmd.length();
				m_block.back().set_flag(1);
			}
			// special case: > $(file)
			auto ge_idx = find_char(single_cmd, '>', 0);
			auto le_idx = find_char(single_cmd, '<', 0);
			if (ge_idx != string::npos&& single_cmd[ge_idx + 1] == ' ')
			{
				m_block.back().set_flag(2);
				m_block.back().set_file(separate_output(single_cmd, ge_idx + 1, single_cmd.length()));
			}

			// send
			if (ge_idx != string::npos && single_cmd[ge_idx + 1] != ' ')
			{
				m_block.back().set_flag(-5);
				m_block.back().set_spec(cmd);
			}
			// receive
			if (le_idx != string::npos)
			{
				m_block.back().set_in(1);
				m_block.back().set_spec(cmd);
			}

			// special case: !N	
			ge_idx = find_char(cmd, '!', 0);
			if (ge_idx != string::npos && end_idx == cmd.length())
			{
				m_block.back().set_flag(0);
				m_block.back().set_cnt(stoi(separate_output(cmd, ge_idx + 1, cmd.length())));
			}	

			// type: builtin
			if (action == "setenv")
			{
				auto space_2 = find_char(single_cmd, ' ', space + 1);
				m_block.back().set_flag(3);
				vector<string> real = {"setenv", separate_output(single_cmd, space + 1, space_2), \
					separate_output(single_cmd, space_2 + 1, single_cmd.length())};
				m_block.back().set_argv(real);
			}
			else if (action == "printenv")
			{
				vector<string> real = {"printenv", separate_output(single_cmd,\
				space + 1, single_cmd.length())};
				m_block.back().set_argv(real);
				m_block.back().set_flag(3);
			}
			else if (action == "exit")
			{
				m_block.back().set_flag(5);
				m_block.back().set_argv({"exit"});
				break;
			}
			// type: bin (4)
			else {
				size_t idx = space, idx2 = space;
				vector<string> argv;
				string temp;
				argv.push_back(action);
				while (idx != string::npos)
				{
					idx2 = find_char(single_cmd, ' ', idx + 1);
					temp = separate_output(single_cmd, idx + 1, idx2);
					idx = idx2;
					if (m_block.back().get_in() && temp.find("<") != string::npos)
						continue;
					argv.push_back(temp);
				}
				if (m_block.back().get_flag() == 0)
					argv.pop_back();
				if (m_block.back().get_flag() == 2)
				{
					argv.pop_back();
					argv.pop_back();
				}
				if (m_block.back().get_flag() == -5)
					argv.pop_back();

				m_block.back().set_argv(argv);
			}			
		}
		start_idx = end_idx + 1;
	}
}
