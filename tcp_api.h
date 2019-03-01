#ifndef TCP_API_H_
#define TCP_API_H_

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
#include <unistd.h>
#include <time.h>

/* defines */

/* function defines */
/* client side */
int tcp_connect(char * , char * );
void tcp_disconnect(int );
int tcp_send_check(int , void *, size_t );
int tcp_recv_check(int , void *, size_t );
/* server side */
int tcp_create(void);
int tcp_accept(int , struct sockaddr_in * );
void tcp_destroy(int );


#endif
