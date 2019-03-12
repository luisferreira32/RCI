#ifndef STRUCTS_H_
#define STRUCTS_H_

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
}peer_conneciton;

#endif
