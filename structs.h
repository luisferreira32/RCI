#ifndef STRUCTS_H_
#define STRUCTS_H_


/* constants */
#define SSBUFFSIZE 10
#define SBUFFSIZE 128
#define MBUFFSIZE 500
#define BBUFFSIZE 1000
#define STDIN 0
#define POPBEGIN 10

/* struct definitions */
/* a struct defining to whom we will connect and general aspects of our own conneciton */
typedef struct iamroot_connection_s
{
    char streamID[63];
    char streamname[54];
    char streamip[16];
    int streamport;
    char ipaddr[15];
    int tport;
    int uport;
    char rsaddr[15];
    int rsport;
    int tcpsessions;
    int bestpops;
    int tsecs;
}iamroot_connection;

/* a struct defining how the client interface will change */
typedef struct client_interface_s
{
    bool debug;
    bool display;
    char format[4];
}client_interface;

/* a struct managing all connections fds, buffers and addresses as well as status of those connections */
typedef struct peer_connection_s
{
    bool amiroot;
    bool interrupted;
    int treeprinter;
    char ** popaddr;
    int popcounter;
    int accessfd;
    int fatherfd;
    char * fatherbuff;
    int recvfd;
    int * childrenfd;
    char ** childrenaddr;
    char ** childbuff;
    int nofchildren;
}peer_conneciton;

/* struct o make a POP query list */
typedef struct pop_list_s
{
    struct pop_list_s * next;
    char queryID[5];
    int bestpops;
}pop_list;

#endif
