#ifndef UDP_API_H_
#define UDP_API_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

/* defines */

/* function defines */
/* general functions */
void udp_destroy(int);
int udp_recv(int , void * , int , struct sockaddr_in * , bool);
int udp_send(int , void * , size_t , struct sockaddr_in * , bool);
/* client side */
int udp_create_client(char * ,char *, int , struct sockaddr_in *);
/* server side */
int udp_server( int );



#endif
