/* own header include */
#include "stream_api.h"

/* functions definitions */

/* basic functions inside the package
    - open port and connect to father
    - open service and create relation with childrenfd
    - read from fd and depending on origin & message propagate /display*/

/* connect to stream and return its file descritor*/
int connect_stream(char * streamip, int streamport)
{
    int sockfd = -1;

    if ((sockfd = tcp_client(NULL, streamip, streamport)) < 0)
    {
        printf("[LOG] Failed to create tcp client\n");
        return -1;
    }
    return sockfd;
}

/* read stream and propagate */
int stream_recv(peer_conneciton* myself, client_interface * my_ci)
{
    char buffer[MBUFFSIZE], header[SSBUFFSIZE];
    int size_recv = 0;

    if ((size_recv = tcp_recv(myself->fatherfd, buffer, MBUFFSIZE, my_ci->debug))<0)
    {
        printf("[LOG] Failed to recieve stream content \n");
    }
    if (sscanf(buffer, "%s ", header)<0)
    {
        perror("[ERROR] Failed to fetch header from stream message ");
    }

    /* depending on header treat it  */
    if (strcmp("DA", header) == 0)
    {
        if (my_ci->display == true)
        {
            printf("%s", buffer);
        }
    }

    return size_recv;
}
