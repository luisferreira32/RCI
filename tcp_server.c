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
INPUT -  server file descriptor
OUTPUT - success
*/
int tcp_create(int * inet_reciv)
{
    struct sockaddr_in inet_source;
    int port = 8000;

    /*opens a stream socket to listen to our applications*/
    if(((*inet_reciv) = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }
    
    /*sets the address of our inet socket and binds it*/
    inet_source.sin_family = AF_INET;
    inet_source.sin_port = htons(port);
    inet_source.sin_addr.s_addr = INADDR_ANY;
    if(bind((*inet_reciv),(const struct sockaddr *) &inet_source, sizeof(inet_source))<0)
    {
        perror("Inet socket bind ");
        return -1;
    }

    printf("The address is %s on port %d \n",inet_ntoa(inet_source.sin_addr),port );
    return 0;
}

/************************************************************************************************/
/**** tcp_destroy **** destroys inet sockets
INPUT - socket file descriptor
OUTPUT - none
DESCRIPTION - closes the socket it created for other clipboards
*/
void tcp_destroy(int fd)
{
    if(close(fd))
    {
        perror("INET socket close ");
    }
}