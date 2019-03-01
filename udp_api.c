/* includes of external libs */

/* own header include */
#include "udp_api.h"

/* functions definitions */

/* GENERAL FUNCTIONS */

/************************************************************************************************/
/**** udp_open **** destroys inet sockets
INPUT - none
OUTPUT - returns socket file descriptor for udp messages
*/
int udp_open(void)
{
    int inet_reciv = -1;

    /*opens a stream socket to listen to our applications*/
    if((inet_reciv = socket(AF_INET, SOCK_DGRAM, 0))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    return inet_reciv;
}

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

    if((bytes_recv= sendto(socket_fd, buf, count, flags, peer, (socketlen_t)sizeof(const struct sockaddr_in)))<0)
    {
        perror("recv udp ");
    }

    if (bytes_recv > count)
    {
        printf("overflowed buffer\n" );
    }

    return bytes_recv;
}

/* SERVER SIDE */

/************************************************************************************************/
/**** udp_create **** for server to open business
INPUT -  none
OUTPUT - socked file descriptor or error -1
*/
int udp_create(void)
{
    struct sockaddr_in server_addr;
    int port = 8000;
    int inet_reciv = -1;

    /*opens a stream socket to listen to our applications*/
    if((inet_reciv = socket(AF_INET, SOCK_DGRAM, 0))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    /*sets the address of our inet socket and binds it*/
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(inet_reciv,(const struct sockaddr *) &server_addr, sizeof(struct sockaddr_in))<0)
    {
        perror("Inet socket bind ");
        return -1;
    }

    return inet_reciv;
}
