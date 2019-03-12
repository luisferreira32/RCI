#ifndef STRUCTS_H_
#define STRUCTS_H_

/* struct definitions */
typedef struct root_connection_s
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
}root_connection;

typedef struct client_interface_s
{
    bool debug;
    bool display;
    char format[4];
}client_interface;

#endif
