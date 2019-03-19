/* own header include */
#include "stream_api.h"

/* functions definitions */

/* basic functions inside the package
    - open port and connect to father
    - open service and create relation with childrenfd
    - read from fd and depending on origin & message propagate /display*/

/* create listening socket */
int recieve_listeners(int accessport)
{
    return tcp_server(accessport);
}

/* connect to stream and return its file descritor*/
int connect_stream(char * streamip, int streamport)
{
    return tcp_client(NULL, streamip, streamport);
}

/* read stream and propagate */
int stream_recv(peer_conneciton* myself, client_interface * my_ci)
{
    char recv_msg[SBUFFSIZE], header[SSBUFFSIZE];
    int size_recv = 0, i = 0;

    /* clear buffer */
    memset(recv_msg,0,SBUFFSIZE);
    /* recieve message from stream parent */
    if ((size_recv = tcp_recv(myself->fatherfd, recv_msg, SBUFFSIZE, my_ci->debug))<0)
    {
        printf("[LOG] Failed to recieve stream content \n");
    }
    if (sscanf(recv_msg, "%s ", header)<0)
    {
        perror("[ERROR] Failed to fetch header from stream message ");
    }

    /* depending on header treat it  */
    /* if it's data */
    if (strcmp("DA", header) == 0)
    {
        /*display*/
        if (my_ci->display == true)
        {
            /*ascii*/
            if (strcmp(my_ci->format, "ascii")==0)
            {
                printf("%s", recv_msg);
            }
            else /*or hex*/
            {
                for (i = 0; i < (int)strlen(recv_msg); i++)
                {
                    printf("%02X", (unsigned int)recv_msg[i]);
                }
                printf("\n");
            }
        }
        /* then resend it to children*/
        for (i = 0; i < myself->nofchildren; i++)
        {
            if (tcp_send(myself->childrenfd[i], recv_msg, strlen(recv_msg),my_ci->debug))
            {
                printf("[LOG] Failed to propagate message to child %d\n", i+1);
            }
        }
    }

    return size_recv;
}
