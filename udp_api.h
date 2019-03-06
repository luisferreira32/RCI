#ifndef UDP_API_H_
#define UDP_API_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

/* defines */

/* function defines */
/* general functions */
void udp_destroy(int);
int udp_recv(int , void * , size_t , int , const struct sockaddr_in * );
int udp_send(int , void * , size_t , int , const struct sockaddr_in * );
/* client side */
int udp_client(char * , int );
/* server side */
int udp_server( int );



#endif UDP_API_H_
