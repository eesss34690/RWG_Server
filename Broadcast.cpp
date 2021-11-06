#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "typedef.hpp"
#include <algorithm>
#include <stdio.h>

int	errexit(const char *format, ...);
using namespace std;

Broadcast::Broadcast()
{
    users.resize(30);
    ip.resize(30);
    ports.resize(30); 
    env.resize(30);
    socket.resize(30);
    in_fd.resize(0);
    out_fd.resize(0); 
    pipes.resize(0);
    smallest = 0;
    cur = 0;
    memset(&sbuf, 0, sizeof sbuf);
}

int Broadcast::add_user(sockaddr_in fsin, int sock)
{
    update_small();
    users[smallest] = "(no name)";
    ip[smallest] = inet_ntoa(fsin.sin_addr);
    ports[smallest] = std::to_string((int)ntohs(fsin.sin_port));
    socket[smallest] = sock;
    env[smallest].push_back(make_pair("PATH","bin:."));
    return smallest;
}

void Broadcast::update_small()
{
    int i = smallest;
    for (i = 0; i < 30; i++)
    {
        if (users[i] == "")
        {
            smallest = i;
            break;
        }
    }
    if (i == 30)
        std::cerr << "out of range users login\n";
}

void Broadcast::welcome(int fd)
{
    strcat(sbuf, "****************************************\n\
** Welcome to the information server. **\n\
****************************************\n\0");
    cout << sizeof sbuf << endl;
    if ((write(fd, sbuf, strlen(sbuf) )) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);
}

void Broadcast::login(int id)
{
    strcat(sbuf, "*** User '");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, "' entered from ");
    strcat(sbuf, ip[id].c_str());
    strcat(sbuf, ":");
    strcat(sbuf, ports[id].c_str());
    strcat(sbuf, ". ***\n\0");
    brst_msg();
}

void Broadcast::logout(int id)
{
    strcat(sbuf, "*** User '");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, "' left. ***\n");
    brst_msg();

}

void Broadcast::brst_msg()
{
    cout << strlen(sbuf) << endl;
    for (auto &i: socket)
    {
        if (i != 0)
        {
            if ((write(i, sbuf, strlen(sbuf) )) < 0)
                errexit ("write error brst\n");
        }
    }
    memset(&sbuf, 0, sizeof sbuf);
}

void Broadcast::delete_user(int id)
{
    users[id] = "";
    ip[id] = "";
    ports[id] = "";
    env[id].clear();
    socket[id] = 0;
    for (int i = 0; i< in_fd.size();)
    {
        if (in_fd[i] == id || out_fd[i] == id)
        {
            in_fd.erase(in_fd.begin() + i);
            out_fd.erase(out_fd.begin() + i);
            pipes.erase(pipes.begin() + i);
        }
        else
            i++;
    }
}

void Broadcast::who(int fd)
{
    strcat(sbuf, "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n");
    for (int i = 0; i < 30; i ++)
    {
        if (users[i] != "")
        {
            strcat(sbuf, std::to_string(i+1).c_str());
            strcat(sbuf, "\t");
            strcat(sbuf, users[i].c_str());
            strcat(sbuf, "\t");
            strcat(sbuf, ip[i].c_str());
            strcat(sbuf, ":");
            strcat(sbuf, ports[i].c_str());
            if (fd == socket[i])
            {
                strcat(sbuf, "\t");
                strcat(sbuf, "<-me");
            }
            strcat(sbuf, "\n");
        }
    }
    if ((write(fd, sbuf, strlen(sbuf))) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);
}

void Broadcast::update_env(string name, string val, int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    bool add = true;
    for (auto &i: env[id])
    {
        if (i.first == name)
        {
            add = false;
            i.second = val;
            break;
        }
    }
    if (add)
        env[id].push_back(make_pair(name, val));
}

void Broadcast::shift_env(int id)
{
    for (auto &i: env[cur])
        ::unsetenv(i.first.c_str());
    cur = id;
    for (auto &i: env[id])
        ::setenv(i.first.c_str(), i.second.c_str(), 1);
}

void Broadcast::name(string new_, int fd)
{
    if (std::find(users.begin(), users.end(), new_) != users.end()) 
    {
        strcat(sbuf, "*** User '");
        strcat(sbuf, new_.c_str());
        strcat(sbuf, "' already exists. ***\n");

        if ((write(fd, sbuf, strlen(sbuf))) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);
    }
    else
    {
        int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
        change_name(new_, id);

        strcat(sbuf, "*** User from ");
        strcat(sbuf, ip[id].c_str());
        strcat(sbuf, ":");
        strcat(sbuf, ports[id].c_str());
        strcat(sbuf, " is named '");
        strcat(sbuf, users[id].c_str());
        strcat(sbuf, "'. ***\n");
        brst_msg();        
    }
}

void Broadcast::tell(string msg, int fd, int to_id)
{
    if (users[to_id - 1] == "") 
    {
        strcat(sbuf, "*** Error: user #");
        strcat(sbuf, std::to_string(to_id).c_str());
        strcat(sbuf, "does not existyet. ***\n");

        if ((write(fd, sbuf, strlen(sbuf))) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);
    }
    else
    {
        int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));

        strcat(sbuf, "*** ");
        strcat(sbuf, users[id].c_str());
        strcat(sbuf, " told you ***: ");
        strcat(sbuf, msg.c_str());
        strcat(sbuf, "\n");
        
        if ((write(socket[to_id - 1], sbuf, strlen(sbuf))) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);        
    }
}
void Broadcast::yell(string msg, int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    strcat(sbuf, "*** ");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, " yelled ***: ");
    strcat(sbuf, msg.c_str());
    strcat(sbuf, "\n");
    brst_msg();
}
int Broadcast::get_out(int fd, string cmd)
{
    auto ge_idx = cmd.find('>', 0) + 1;
    auto space = cmd.find(' ', ge_idx);
    
    while (cmd[ge_idx] == ' ')
		ge_idx++;
	while (cmd[--space] == ' ');
	int id_to = std::stoi(cmd.substr(ge_idx, 1 + space - ge_idx)) - 1;
    int id_fm = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));

    if (id_to > 29 || id_fm > 29 || users[id_fm] == "" || users[id_to] == "")
    {
        strcat(sbuf, "*** Error: user #");
        strcat(sbuf, std::to_string(id_to + 1).c_str());
        strcat(sbuf, " does not exist yet. ***\n");
        if ((write(fd, sbuf, strlen(sbuf))) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);   
        return -1;
    }

    bool cont = true;
    for (int i = 0; i < in_fd.size(); i++)
    {
        cout << "searching for " << in_fd[i] << " to " << out_fd[i] << endl;
        // Do something with iter
        if (out_fd[i] == id_to && in_fd[i] == id_fm)
        {
            strcat(sbuf, "*** Error: the pipe #");
            strcat(sbuf, std::to_string(id_fm + 1).c_str());
            strcat(sbuf, "->#");
            strcat(sbuf, std::to_string(id_to + 1).c_str());
            strcat(sbuf, " already exists. ***\n");
            cont = false;
            if ((write(fd, sbuf, strlen(sbuf))) < 0)
                errexit ("write error brst\n");
            memset(&sbuf, 0, sizeof sbuf);   
            return -1;
        }
    }
    if (cont)
    {
        Pipe_IO pipe;
        pipe = Pipe_IO::create();
        pipes.push_back(pipe);
        in_fd.push_back(id_fm);
        out_fd.push_back(id_to);
        strcat(sbuf, "*** ");
        strcat(sbuf, users[id_fm].c_str());
        strcat(sbuf, " (#");
        strcat(sbuf, std::to_string(id_fm + 1).c_str());
        strcat(sbuf, ") just piped '");
        strcat(sbuf, cmd.c_str());
        strcat(sbuf, "' to ");
        strcat(sbuf, users[id_to].c_str());
        strcat(sbuf, " (#");
        strcat(sbuf, std::to_string(id_to + 1).c_str());
        strcat(sbuf, ") ***\n");
        brst_msg();
        memset(&sbuf, 0, sizeof sbuf); 
        return pipe.get_out();
    }
}

int Broadcast::get_in(string cmd, int fd)
{
    auto ge_idx = cmd.find('<', 0) + 1;
    auto space = cmd.find(' ', ge_idx);
    
    while (cmd[ge_idx] == ' ')
		ge_idx++;
	while (cmd[--space] == ' ');
	int id_fm = std::stoi(cmd.substr(ge_idx, 1 + space - ge_idx)) - 1;
    int id_to = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));

    if (id_to > 29 || id_fm > 29 || users[id_fm] == "" || users[id_to] == "")
    {
        strcat(sbuf, "*** Error: user #");
        strcat(sbuf, std::to_string(id_fm + 1).c_str());
        strcat(sbuf, " does not exist yet. ***\n");
        if ((write(fd, sbuf, strlen(sbuf))) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);   
        return -1;
    }

    for (int i = 0; i < in_fd.size(); i++)
    {
        cout << "searching for " << in_fd[i] << " to " << out_fd[i] << endl;
        // Do something with iter
        if (out_fd[i] == id_to && in_fd[i] == id_fm)
        {
            strcat(sbuf, "*** ");
            strcat(sbuf, users[id_to].c_str());
            strcat(sbuf, " (#");
            strcat(sbuf, std::to_string(id_to + 1).c_str());
            strcat(sbuf, ") just received from ");
            strcat(sbuf, users[id_fm].c_str());
            strcat(sbuf, " (#");
            strcat(sbuf, std::to_string(id_fm + 1).c_str());
            strcat(sbuf, ") by '");
            strcat(sbuf, cmd.c_str());
            strcat(sbuf, "' ***\n");
            brst_msg();
            memset(&sbuf, 0, strlen(sbuf)); 

            int temp = pipes[i].get_in();

            in_fd.erase(in_fd.begin() + i);
            out_fd.erase(out_fd.begin() + i);
            pipes.erase(pipes.begin() + i);
            return temp;
        }
    }

    strcat(sbuf, "*** Error: the pipe #");
    strcat(sbuf, std::to_string(id_fm + 1).c_str());
    strcat(sbuf, "->#");
    strcat(sbuf, std::to_string(id_to + 1).c_str());
    strcat(sbuf, " does not exist yet. ***\n");
    if ((write(fd, sbuf, strlen(sbuf))) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);  
             
    return -1;
    
}


