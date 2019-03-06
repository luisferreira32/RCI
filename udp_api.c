/* own header include */
#include "udp_api.h"

/* functions definitions */

/* GENERAL FUNCTIONS */

/************************************************************************************************/
/**** udp_destroy **** destroys inet sockets
INPUT - socket file descriptor
OUTPUT - none
*/
void udp_destroy(int fd)
{
    if(close(fd))
    {
        perror("INET socket close ");
    }
}

/************************************************************************************************/
/**** udp_send **** sends datagram
INPUT - socket file descriptor buffer size and flags
OUTPUT - bytes sent
*/
int udp_send(int socket_fd, void * buf, size_t count, int flags, const struct sockaddr_in * peer)
{
    int bytes_sent = 0;

    if((bytes_sent= sendto(socket_fd, buf, count, flags, peer, (socketlen_t)sizeof(const struct sockaddr_in)))<0)
    {
        perror("send udp ");
    }

    return bytes_sent;
}

/************************************************************************************************/
/**** udp_recv **** recieves datagram
INPUT - socket file descriptor buffer size and flags
OUTPUT - bytes sent
*/
int udp_recv(int socket_fd, void * buf, size_t count, int flags, const struct sockaddr_in * peer)
{
    int bytes_recv = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if((bytes_recv= recvfrom(socket_fd, buf, count, flags, peer, &addrlen))<0)
    {
        perror("recv udp ");
    }

    if( sizeof(struct sockaddr_in) != addrlen)
    {
        printf("Unexpected socket address\n" );
    }

    if (bytes_recv > count)
    {
        printf("overflowed buffer\n" );
    }

    return bytes_recv;
}

/* CLIENT EXCLUSIVE */

/************************************************************************************************/
/**** udp_client **** destroys inet sockets
INPUT - dns and port, and a pointer to the "peer"
OUTPUT - returns socket file descriptor for udp messages
*/
int udp_client(char * dns, int port, struct addrinfo * peer)
{
    int inet_reciv = -1;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_NUMERICSERV;

    if (getaddrinfo (dns,port,&hints,&res))
    {
        perror("Get socket addr info ");
        return -1;
    }

    if((inet_reciv = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    *peer = *res;
    freeaddrinfo(res);
    return inet_reciv;
}

/* SERVER EXCLUSIVE */

/************************************************************************************************/
/**** udp_server **** for server to open business
INPUT -  none
OUTPUT - socked file descriptor or error -1
*/
int udp_server(int port)
{
    int inet_reciv = -1;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;

    if (getaddrinfo (NULL,port,&hints,&res))
    {
        perror("Get socket addr info ");
        return -1;
    }

    if((inet_reciv = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    if(bind(inet_reciv,res->ai_addr,res->ai_addrlen) == -1 )
    {
        perror("Failed bind ");
        return -1;
    }

    freeaddrinfo(res);
    return inet_reciv;
}
