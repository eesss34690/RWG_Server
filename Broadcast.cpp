#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "typedef.hpp"

int	errexit(const char *format, ...);

Broadcast::Broadcast()
{
    users.resize(30);
    ip.resize(30);
    ports.resize(30); 
    socket.resize(30);
    smallest = 0;
    memset(&sbuf, 0, sizeof sbuf);
}

int Broadcast::add_user(sockaddr_in fsin, int sock)
{
    update_small();
    users[smallest] = "(no name)";
    ip[smallest] = inet_ntoa(fsin.sin_addr);
    ports[smallest] = std::to_string((int)ntohs(fsin.sin_port));
    socket[smallest] = sock;
    return smallest;
}

void Broadcast::update_small()
{
    int i = smallest;
    for (i = smallest; i < 30; i++)
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
****************************************\n");
    if ((write(fd, sbuf, sizeof sbuf)) < 0)
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
    strcat(sbuf, ". ***\n");
    brst_msg();
}

void Broadcast::logout(int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    strcat(sbuf, "*** User '");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, "' left. ***\n");
    brst_msg();
}

void Broadcast::brst_msg()
{
    for (auto &i: socket)
    {
        if (i != 0)
        {
            if ((write(i, sbuf, sizeof sbuf)) < 0)
                errexit ("write error brst\n");
        }
    }
    memset(&sbuf, 0, sizeof sbuf);
}

void Broadcast::delete_user(int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    users[id] = "";
    ip[id] = "";
    ports[id] = "";
    socket[id] = 0;
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
                strcat(sbuf, "<-me");
            strcat(sbuf, "\n");
        }
    }
    if ((write(fd, sbuf, sizeof sbuf)) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);
}

void Broadcast::name(string new_, int fd)
{
    if (std::find(users.begin(), users.end(), new_) != users.end()) 
    {
        strcat(sbuf, "*** User '");
        strcat(sbuf, new_.c_str());
        strcat(sbuf, "' already exists. ***\n");

        if ((write(fd, sbuf, sizeof sbuf)) < 0)
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