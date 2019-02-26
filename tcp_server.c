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

/************************************************************************************************/
/**** tcp_create **** for server to open business
INPUT -  none
OUTPUT - socked file descriptor or error -1
*/
int tcp_create(void)
{
    struct sockaddr_in server_addr;
    int port = 8000;
    int inet_reciv = -1;

    /*opens a stream socket to listen to our applications*/
    if((inet_reciv = socket(AF_INET, SOCK_STREAM, 0))==-1)
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

    //printf("The address is %s on port %d \n",inet_ntoa(server_addr.sin_addr),port );
    return inet_reciv;
}

/************************************************************************************************/
/**** tcp_create **** for server to open business
INPUT -  server file descriptor, pointer to memory for client address
OUTPUT - new socket file descriptor for client communicaiton
*/
int tcp_accept(int server_fd, struct sockaddr_in * client_addr)
{
    socklen_t client_addr_size;
    int client_fd = -1;

    /*accept incoming connecion and save client address*/
    client_addr_size = sizeof(struct sockaddr_in);
    if(accept(server_fd, (struct sockaddr_in *) client_addr, &client_addr_size)<0)
    {
        perror("Failed to accept connection ");
        return -1;
    }
    /*check if address is complete*/
    if(client_addr_size < sizeof(struct sockaddr_in))
    {
        printf("Incomplete client address \n");
    }

    return client_fd;
}

/************************************************************************************************/
/**** tcp_destroy **** destroys inet sockets
INPUT - socket file descriptor
OUTPUT - none
*/
void tcp_destroy(int fd)
{
    if(close(fd))
    {
        perror("INET socket close ");
    }
}
