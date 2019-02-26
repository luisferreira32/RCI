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

    //printf("The address is %s on port %d \n",inet_ntoa(server_addr.sin_addr),port );
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
