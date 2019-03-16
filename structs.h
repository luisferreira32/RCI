#ifndef STRUCTS_H_
#define STRUCTS_H_


/* constants */
#define SSBUFFSIZE 10
#define SBUFFSIZE 100
#define MBUFFSIZE 500
#define BBUFFSIZE 1000
#define STDIN 0

/* struct definitions */
typedef struct iamroot_connection_s
{
    char streamname[54];
    char streamip[15];
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

typedef struct client_interface_s
{
    bool debug;
    bool display;
    char format[4];
}client_interface;

typedef struct peer_connection_s
{
    bool amiroot;
    int accessfd;
    int fatherfd;
    int * childrenfd;
    int nofchildren;
}peer_conneciton;

#endif
