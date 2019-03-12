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
        perror("[LOG] INET socket close ");
    }
}

/************************************************************************************************/
/**** udp_send **** sends datagram
INPUT - socket file descriptor buffer size and flags
OUTPUT - bytes sent
*/
int udp_send(int socket_fd, void * buf, size_t count, const struct sockaddr_in * peer, bool debug)
{
    int bytes_sent = 0;

    /* send datagram and check fo errors */
    if((bytes_sent= sendto(socket_fd, buf, count, MSG_CONFIRM, peer, (socketlen_t)sizeof(const struct sockaddr_in)))<0)
    {
        perror("[LOG] send udp ");
    }

    if (debug == true)
    {
        printf("[DEBUG] UDP message of %d bytes sent\n", bytes_sent);
    }

    return bytes_sent;
}

/************************************************************************************************/
/**** udp_recv **** recieves datagram
INPUT - socket file descriptor buffer size and flags
OUTPUT - bytes sent
*/
int udp_recv(int socket_fd, void * buf, size_t count, const struct sockaddr_in * peer, bool debug)
{
    int bytes_recv = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    /* recieve message */
    if((bytes_recv= recvfrom(socket_fd, buf, count, MSG_WAITALL, peer, &addrlen))<0)
    {
        perror("[LOG] recv udp ");
    }

    /* treat basic anomalies */
    if(sizeof(struct sockaddr_in) != addrlen)
    {
        printf("[LOG] Unexpected socket address\n" );
    }

    if(bytes_recv > count)
    {
        printf("[LOG] overflowed buffer\n" );
    }

    buf[bytes_recv] = '\0';

    if(debug == true)
    {
        printf("[DEBUG] UDP message with %d bytes recieved\n", bytes_recv);
    }

    return bytes_recv;
}

/* CLIENT EXCLUSIVE */

/************************************************************************************************/
/**** udp_create_client **** creates inet sockets
INPUT - dns OR ip AND port, and a pointer to the "peer"
OUTPUT - returns socket file descriptor for udp messages
*/
int udp_create_client(char * dns, char * ip, int port, struct sockaddr_in * peer)
{
    int inet_reciv = -1;
    struct addrinfo hints,*res;

    /* connect based on DNS*/
    if(ip == NULL && dns != NULL)
    {
        memset(&hints,0,sizeof(hints));
        hints.ai_family=AF_INET;
        hints.ai_socktype=SOCK_DGRAM;
        hints.ai_flags=AI_NUMERICSERV;

        if (getaddrinfo (dns,port,&hints,&res))
        {
            perror("[LOG] Get socket addr info ");
            return -1;
        }

        *peer = res->ai_addr;
        freeaddrinfo(res);
    }
    /* connect based on IP */
    else if(ip != NULL)
    {
        hints.sin_family = AF_INET;
        hints.sin_port = htons(59000);
        if(inet_aton(ip, peer->sin_addr)==0)
        {
            printf("[LOG] Invalid IP address %s", ip);
            return -1;
        }
    }
    else
    {
        printf("[LOG] Need IP or DNS for UDP connection\n", );
        return -1;
    }

    if((inet_reciv = socket(AF_INET, SOCK_DGRAM, 0))==-1)
    {
        perror("[LOG] Inet socket creation ");
        return -1;
    }

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
        perror("[LOG] Get socket addr info ");
        return -1;
    }

    if((inet_reciv = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
    {
        perror("[LOG] Inet socket creation ");
        return -1;
    }

    if(bind(inet_reciv,res->ai_addr,res->ai_addrlen) == -1 )
    {
        perror("[LOG] Failed bind ");
        return -1;
    }

    freeaddrinfo(res);
    return inet_reciv;
}
