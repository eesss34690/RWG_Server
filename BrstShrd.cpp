#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "typedef.hpp"
#include <algorithm>
#include <stdio.h>

int	    errexit(const char *format, ...);
int		sem_create(key_t, int);
void	sem_wait(int);
void	sem_signal(int);
using namespace std;

BrstShrd::BrstShrd()
{
    users.resize(30);
    ip.resize(30);
    ports.resize(30); 
    env.resize(30);
    socket.resize(30);
    in_user.resize(0);
    out_user.resize(0); 
    in_fd.resize(0); 
    out_fd.resize(0); 
    pipes.resize(0);
    smallest = 0;
    cur = 0;
    key1 = 8891;
    memset(&sbuf, 0, sizeof sbuf);
    if ( (clisem = sem_create(key1, 1)) < 0) 
        errexit("...");
}

int BrstShrd::add_user(sockaddr_in fsin, int sock)
{
    sem_wait(clisem);
    update_small();
    users[smallest] = "(no name)";
    ip[smallest] = inet_ntoa(fsin.sin_addr);
    ports[smallest] = std::to_string((int)ntohs(fsin.sin_port));
    socket[smallest] = sock;
    env[smallest].push_back(make_pair("PATH","bin:."));
    int temp = smallest;
    sem_signal(clisem);
    return temp;
}

void BrstShrd::update_small()
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
        errexit("out of range users login\n");
}

void BrstShrd::welcome(int fd)
{
    strcat(sbuf, "****************************************\n\
** Welcome to the information server. **\n\
****************************************\n\0");
    if ((write(fd, sbuf, strlen(sbuf) )) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);
}

void BrstShrd::login(int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    strcat(sbuf, "*** User '");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, "' entered from ");
    strcat(sbuf, ip[id].c_str());
    strcat(sbuf, ":");
    strcat(sbuf, ports[id].c_str());
    strcat(sbuf, ". ***\n\0");
    brst_msg();
}

void BrstShrd::logout(int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    strcat(sbuf, "*** User '");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, "' left. ***\n");
    brst_msg();

}

void BrstShrd::brst_msg()
{
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

void BrstShrd::delete_user(int fd)
{
    sem_wait(clisem);
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    users[id] = "";
    ip[id] = "";
    ports[id] = "";
    for (auto &i: env[id])
        ::unsetenv(i.first.c_str());
    env[id].clear();
    socket[id] = 0;
    for (int i = 0; i< in_user.size();)
    {
        if (in_user[i] == id || out_user[i] == id)
        {
            in_user.erase(in_user.begin() + i);
            out_user.erase(out_user.begin() + i);
            in_fd.erase(in_fd.begin() + i);
            out_fd.erase(out_fd.begin() + i);
            pipes.erase(pipes.begin() + i);
        }
        else
            i++;
    }
    sem_signal(clisem);
}

void BrstShrd::who(int fd)
{
    sem_wait(clisem);
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
    sem_signal(clisem);
}

void BrstShrd::update_env(string name, string val, int fd)
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

void BrstShrd::shift_env(int fd)
{
    sem_wait(clisem);
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    for (auto &i: env[cur])
        ::unsetenv(i.first.c_str());
    cur = id;
    for (auto &i: env[id])
        ::setenv(i.first.c_str(), i.second.c_str(), 1);
    sem_signal(clisem);
}

void BrstShrd::name(string new_, int fd)
{
    sem_wait(clisem);
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
    sem_signal(clisem);  
}

void BrstShrd::tell(string msg, int fd, int to_id)
{
    sem_wait(clisem);
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
    sem_signal(clisem);
}
void BrstShrd::yell(string msg, int fd)
{
    sem_wait(clisem);
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    strcat(sbuf, "*** ");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, " yelled ***: ");
    strcat(sbuf, msg.c_str());
    strcat(sbuf, "\n");
    brst_msg();
    sem_signal(clisem);
}

int BrstShrd::get_out(int fd, string cmd)
{
    sem_wait(clisem);
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
        
        sem_signal(clisem); 
        return -1;
    }

    bool cont = true;
    for (int i = 0; i < in_user.size(); i++)
    {
        cout << "searching for " << in_user[i] << " to " << out_user[i] << endl;
        // Do something with iter
        if (out_user[i] == id_to && in_user[i] == id_fm)
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
            sem_signal(clisem);
            return -1;
        }
    }
    if (cont)
    {
        string temp = "./user_pipe/" + std::to_string(id_fm) + "_" + std::to_string(id_to);
        cout << temp << endl;
        if ( (mkfifo(temp.c_str(), 0666) < 0) && (errno != EEXIST) )
            errexit("can't create fifo 1: %s", temp.c_str()); 
        pipes.push_back(temp);
        in_user.push_back(id_fm);
        out_user.push_back(id_to);
	cout << "ready to connect\n";
        raise(SIGUSR1);
        cout << "finished all\n";
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
        int fdin = in_fd.back();
        sem_signal(clisem);
        return fdin;
    }
}

int BrstShrd::get_in(string cmd, int fd)
{
    sem_wait(clisem);
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
        sem_signal(clisem);
        return -1;
    }

    for (int i = 0; i < in_user.size(); i++)
    {
        cout << "searching for " << in_user[i] << " to " << out_user[i] << endl;
        // Do something with iter
        if (out_user[i] == id_to && in_user[i] == id_fm)
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
            
            int readfd = out_fd[i];
            in_user.erase(in_user.begin() + i);
            out_user.erase(out_user.begin() + i);
            in_fd.erase(in_fd.begin() + i);
            out_fd.erase(out_fd.begin() + i);
            pipes.erase(pipes.begin() + i);
            cout << readfd << endl;
            sem_signal(clisem);
            return readfd;
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
    sem_signal(clisem);
             
    return -1;
    
}


