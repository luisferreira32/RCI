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
