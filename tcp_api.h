#ifndef TCP_API_H_
#define TCP_API_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
/* client side */
int tcp_client(char * ,char * , int);
/* general functions */
int tcp_send(int , void *, size_t, bool );
int tcp_recv(int , void *, size_t, bool );
void tcp_disconnect(int );
/* server side */
int tcp_server(int);
int tcp_accept(int , struct sockaddr_in * );


#endif
