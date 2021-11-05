#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "typedef.hpp"
#include <algorithm>
#include <sys/ipc.h>
#include <sys/shm.h>

int	errexit(const char *format, ...);
int sem_create(key_t key, int initval);
void sem_wait(int id);
void sem_signal(int id);
using namespace std;

BrstShrd::BrstShrd()
{
    char c;
    int shmid;
    shr_key1 = 8883;
    shr_key2 = 8884;
    shr_key3 = 8885;
    shr_key4 = 8886;
    shr_key5 = 8887;
    shr_key6 = 8888;
    shr_key7 = 8889;
    sem_key1 = 8890;
    sem_key2 = 8891;

    // shared memory
    if ((shmid = shmget(shr_key1, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((users = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        users->mesg_len = 0;
        memset(&(users->mesg_data), 0, sizeof users->mesg_len);
    }
    // shared memory
    if ((shmid = shmget(shr_key2, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((ip = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        ip->mesg_len = 0;
        memset(&(ip->mesg_data), 0, sizeof ip->mesg_len);
    }
    // shared memory
    if ((shmid = shmget(shr_key3, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((port = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        port->mesg_len = 0;
        memset(&(port->mesg_data), 0, sizeof port->mesg_len);
    }
    // shared memory
    if ((shmid = shmget(shr_key4, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((socket = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        socket->mesg_len = 0;
        memset(&(socket->mesg_data), 0, sizeof socket->mesg_len);
    }
    // shared memory
    if ((shmid = shmget(shr_key5, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((in_fd = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        in_fd->mesg_len = 0;
        memset(&(in_fd->mesg_data), 0, sizeof in_fd->mesg_len);
    }
    // shared memory
    if ((shmid = shmget(shr_key6, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((out_fd = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        out_fd->mesg_len = 0;
        memset(&(out_fd->mesg_data), 0, sizeof out_fd->mesg_len);
    }
    // shared memory
    if ((shmid = shmget(shr_key7, 15001, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((pipes = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
        perror("shmat");
        exit(1);
    }
    else
    {
        pipes->mesg_len = 0;
        memset(&(pipes->mesg_data), 0, sizeof pipes->mesg_len);
    }
    // semaphore
    if ( (clisem = sem_create(sem_key1, 1)) < 0) 
        errexit("...");
    if ( (servsem = sem_create(sem_key2, 0)) < 0) 
        errexit("...");
}

int BrstShrd::add_user(sockaddr_in fsin, int sock)
{
    int	n, filefd;
    char	errmesg[256], *sys_err_str();

    // Wait for the client to write the filename 
    // into shared memory.
    sem_wait(clisem);	/* Will wait here for client to start */
    std::cout << users->mesg_len << "a\n";
    int i = 1, idx = 0, cont = 0;
    if (users->mesg_len == 0)
    {
        strcpy(users->mesg_data, "(no name)");
        users->mesg_len = 10;
    }
    for (; i < users->mesg_len; i++)
    {
        if (users->mesg_data[i-1] == '\0' && users->mesg_data[i] == '\0')
        {
            memmove(users->mesg_data + i + 10, users->mesg_data + i, users->mesg_len - i - 10);
            users->mesg_data[i++] = '(';
            cout <<i << users->mesg_data[i-1];
            users->mesg_data[i++] = 'n';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = '0';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = ' ';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = 'n';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = 'a';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = 'm';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = 'e';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i++] = ')';
            cout <<i  << users->mesg_data[i-1];
            users->mesg_data[i] = '\0';
            cont = 1;
            break;
        }
        else if (users->mesg_data[i-1] != '\0' && users->mesg_data[i] == '\0')
            idx++;
    }
    if (cont == 0)
    {
        ++i;
        users->mesg_data[i++] = '(';
        cout <<i << users->mesg_data[i-1];
        users->mesg_data[i++] = 'n';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = '0';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = ' ';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = 'n';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = 'a';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = 'm';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = 'e';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i++] = ')';
        cout <<i  << users->mesg_data[i-1];
        users->mesg_data[i] = '\0';
    }


    users->mesg_len = (users->mesg_len > i)? users->mesg_len: i;
    sem_signal(clisem);
    cout << users->mesg_data <<"a\n";
    //ip[smallest] = inet_ntoa(fsin.sin_addr);
    //ports[smallest] = std::to_string((int)ntohs(fsin.sin_port));
    //socket[smallest] = sock;
    return idx;
}
/*
void BrstShrd::welcome(int fd)
{
    strcat(sbuf, "****************************************\n\
** Welcome to the information server. **\n\
****************************************\n");
    if ((write(fd, sbuf, sizeof sbuf)) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);
}

void BrstShrd::login(int id)
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

void BrstShrd::logout(int id)
{
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
            if ((write(i, sbuf, sizeof sbuf)) < 0)
                errexit ("write error brst\n");
        }
    }
    memset(&sbuf, 0, sizeof sbuf);
}

void BrstShrd::delete_user(int id)
{
    users[id] = "";
    ip[id] = "";
    ports[id] = "";
    socket[id] = 0;
}

void BrstShrd::who(int fd)
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

void BrstShrd::name(string new_, int fd)
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

void BrstShrd::tell(string msg, int fd, int to_id)
{
    if (users[to_id - 1] == "") 
    {
        strcat(sbuf, "*** Error: user #");
        strcat(sbuf, std::to_string(to_id).c_str());
        strcat(sbuf, "does not existyet. ***\n");

        if ((write(fd, sbuf, sizeof sbuf)) < 0)
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
        
        if ((write(socket[to_id - 1], sbuf, sizeof sbuf)) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);        
    }
}
void BrstShrd::yell(string msg, int fd)
{
    int id = std::distance(socket.begin(), std::find(socket.begin(), socket.end(), fd));
    strcat(sbuf, "*** ");
    strcat(sbuf, users[id].c_str());
    strcat(sbuf, " yelled ***: ");
    strcat(sbuf, msg.c_str());
    strcat(sbuf, "\n");
    brst_msg();
}
string BrstShrd::get_out(int fd, string cmd)
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
        strcat(sbuf, std::to_string(id_to).c_str());
        strcat(sbuf, " does not exist yet. ***\n");
        if ((write(fd, sbuf, sizeof sbuf)) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);   
        return "";
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
            if ((write(fd, sbuf, sizeof sbuf)) < 0)
                errexit ("write error brst\n");
            memset(&sbuf, 0, sizeof sbuf);   
            return "";
        }
    }
    if (cont)
    {
        // construct fifo
        string cur = "/tmp/"+std::to_string(pipes.size() + 1);
        if ( (mkfifo(cur.c_str(), 0666) < 0) && (errno != EEXIST) )
            errexit("can't create fifo 1: %s", cur.c_str()); 
        pipes.push_back(cur);
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
        return cur;
    }
}

string BrstShrd::get_in(string cmd, int fd)
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
        strcat(sbuf, std::to_string(id_to).c_str());
        strcat(sbuf, " does not exist yet. ***\n");
        if ((write(fd, sbuf, sizeof sbuf)) < 0)
            errexit ("write error brst\n");
        memset(&sbuf, 0, sizeof sbuf);   
        return "";
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
            strcat(sbuf, std::to_string(id_fm + 1).c_str());
            strcat(sbuf, ") just received from ");
            strcat(sbuf, users[id_fm].c_str());
            strcat(sbuf, " (#");
            strcat(sbuf, std::to_string(id_to + 1).c_str());
            strcat(sbuf, ") by '");
            strcat(sbuf, cmd.c_str());
            strcat(sbuf, "' ***\n");
            brst_msg();
            memset(&sbuf, 0, sizeof sbuf); 

            string temp = pipes[i];

            in_fd.erase(in_fd.begin() + i);
            out_fd.erase(out_fd.begin() + i);
            pipes.erase(pipes.begin() + i);
            return temp;
        }
    }

    strcat(sbuf, "*** Error: the pipe #");
    strcat(sbuf, std::to_string(id_fm).c_str());
    strcat(sbuf, "->#");
    strcat(sbuf, std::to_string(id_to).c_str());
    strcat(sbuf, " does not exist yet. ***\n");
    if ((write(fd, sbuf, sizeof sbuf)) < 0)
        errexit ("write error brst\n");
    memset(&sbuf, 0, sizeof sbuf);  
             
    return "";
    
}

*/