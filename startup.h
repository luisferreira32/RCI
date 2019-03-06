#ifndef STARTUP_H_
#define STARTUP_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* defines */
#define TCPPORT 58000
#define UDPPORT 58000
#define ROOTIP "193.136.138.142"
#define UDPROOTPORT 59000

/* struct definitions */
typedef struct root_connection_s
{
    char streamname[45];
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
    bool bopt;
    bool dopt;
    bool hopt;
}root_connection;

/* funcitons definitios */
int connect_to_stream(root_connection *, int, const char **);


#endif
