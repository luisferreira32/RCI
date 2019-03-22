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

/* create a fd for a child */
int accept_children(int recvfd, char * addr)
{
    struct sockaddr_in addrstruct;
    int fd = -1, port = -1;
    char * buffer;

    fd = tcp_accept(recvfd, &addrstruct);
    buffer = inet_ntoa(addrstruct.sin_addr);
    port = ntohs(addrstruct.sin_port);
    sprintf(addr, "%s:%d", buffer, port);
    return fd;
}

/* connect to stream and return its file descritor*/
int connect_stream(char * streamip, int streamport)
{
    return tcp_client(NULL, streamip, streamport);
}

/* read stream and propagate */
int stream_recv_downstream(peer_conneciton* myself, client_interface * my_ci, iamroot_connection * my_connect)
{
    /* variables */
    char recv_msg[SBUFFSIZE-SSBUFFSIZE], header[SSBUFFSIZE], capsule[SBUFFSIZE], size[5],data[SBUFFSIZE];
    char * extra_data, *extra_capsule;
    int size_recv = 0, i = 0, data_size = 0;

    /* clear buffer */
    memset(recv_msg,0,SBUFFSIZE);
    /* recieve message from stream parent */
    if ((size_recv = tcp_recv(myself->fatherfd, recv_msg, SBUFFSIZE-SSBUFFSIZE, my_ci->debug))<0)
    {
        printf("[LOG] Failed to recieve stream content \n");
    }
    else if (size_recv == 0)
    {
        return 0;
    }

    /* if i'm root it can only be DATA, let's capsule it and resend*/
    memset(capsule, 0, SBUFFSIZE);
    if (myself->amiroot == true)
    {
        if (sprintf(capsule, "DA %04X\n%s", (unsigned int)strlen(recv_msg),recv_msg)<0)
        {
            perror("[ERROR] Failed to encapsule ");
        }
    }
    else
    {
        strcpy(capsule, recv_msg);
    }

    /* check according to header the procedure */
    memset(header, 0, SSBUFFSIZE);
    if (sscanf(capsule, "%s ", header)!=1)
    {
        perror("[ERROR] Failed to fetch header from stream message ");
    }

    /* depending on header treat it  */
    /* can only be data if the stream is not broken */
    if (strcmp("DA", header) == 0 && myself->interrupted == false)
    {
        /*read the data*/
        if (sscanf(capsule, "%s %[^\n] %[^\n]", header, size, data) != 3)
        {
            printf("[LOG] Failed to fetch data \n");
            return -1;
        }
        data_size = strtol(size, NULL, 16);
        /* if data_size is larger than data recieved */
        if ((data_size > size_recv - 8 && myself->amiroot == false)||(data_size > size_recv && myself->amiroot == true))
        {
            /* code */
        }
        else
        {
            extra_data = data;
            extra_capsule = capsule;
        }
        /*display*/
        if (my_ci->display == true)
        {
            /*ascii*/
            if (strcmp(my_ci->format, "ascii")==0)
            {
                printf("%s\n", extra_data);
            }
            else /*or hex*/
            {
                for (i = 0; i < (int)strlen(extra_data); i++)
                {
                    printf("%02X", (unsigned int)extra_data[i]);
                }
                printf("\n");
            }
        }
        /* then resend it to children*/
        for (i = 0; i < myself->nofchildren; i++)
        {
            if (tcp_send(myself->childrenfd[i], extra_capsule, strlen(extra_capsule),my_ci->debug))
            {
                printf("[LOG] Failed to propagate message to child %d\n", i+1);
            }
        }
        /* and free temp memory if had any */
        if ((data_size > size_recv - 8 && myself->amiroot == false)||(data_size > size_recv && myself->amiroot == true))
        {
            free(extra_data);
            free(extra_capsule);
        }
    }
    /* a welcome message after joining the stream */
    else if(strcmp(header, "WE") == 0)
    {
        memset(capsule, 0, SBUFFSIZE);
        if (sprintf(capsule, "NP %s:%d\n", my_connect->ipaddr, my_connect->tport)<0)
        {
            perror("[ERROR] Failed to formulate NP message ");
        }
        if (tcp_send(myself->fatherfd, capsule, strlen(capsule), my_ci->debug))
        {
            printf("[LOG] Failed to reply to WELCOME message\n");
        }
    }
    /* a redirect message after joining a full spot*/
    else if(strcmp(header, "RE"))
    {
        if (sscanf(capsule, "%s %[^:]:%d", header, my_connect->streamip, &(my_connect->streamport)) != 3)
        {
            perror("[ERROR] Redirect message read ");
        }
        /* disconnect from previous and reconnect */
        tcp_disconnect(myself->fatherfd);
        myself->fatherfd = -1;

        if ((myself->fatherfd = connect_stream(my_connect->streamip, my_connect->streamport)) <0)
        {
            printf("[LOG] Failed to connect to new stream source\n");
            return -1;
        }
    }
    /* check if stream is broken or not */
    else if(strcmp(header, "SF")==0)
    {
        myself->interrupted = false;
    }
    else if(strcmp(header, "BS")==0)
    {
        myself->interrupted = true;
    }
    else
    {
        printf("[LOG] Protocol not followed by father\n");
    }

    return size_recv;
}

/* upstream message treatment */
int stream_recv_upstream(int childfd, peer_conneciton* myself, iamroot_connection * my_connect, bool debug)
{
    /* variables */
    char recv_msg[SBUFFSIZE], header[SSBUFFSIZE];
    int size_recv = 0;

    /* clear buffer */
    memset(recv_msg,0,SBUFFSIZE);
    /* recieve message from stream parent */
    if ((size_recv = tcp_recv(childfd, recv_msg, SBUFFSIZE, debug))<0)
    {
        printf("[LOG] Failed to recieve stream content \n");
    }
    else if (size_recv == 0)
    {
        return 0;
    }

    /* check according to header the procedure */
    memset(header, 0, SSBUFFSIZE);
    if (sscanf(recv_msg, "%s ", header) ==0)
    {
        perror("[ERROR] Failed to fetch header from stream message ");
    }

    /* if it's a new pop i must save it for possible redirect*/
    if (strcmp(header, "NP")==0)
    {
        if (myself->popcounter > my_connect->bestpops || sscanf(recv_msg, "%s %s\n", header, myself->ipaddrtport[myself->popcounter])!=2)
        {
            printf("[LOG] Did not add NP\n" );
        }
        else
        {
            myself->popcounter++;
        }
    }

    return size_recv;

}

/* welcoming message */
int stream_welcome(iamroot_connection * my_connect, peer_conneciton * myself, bool debug)
{
    char message[SBUFFSIZE];

    /* welcome and stream info */
    if (sprintf(message, "WE %s\n", my_connect->streamID)<0)
    {
        perror("[ERROR] Failed to formulate welcome message ");
        return -1;
    }
    if (tcp_send(myself->childrenfd[myself->nofchildren], message, strlen(message), debug))
    {
        return -1;
    }
    /*strea status*/
    memset(message,0,SBUFFSIZE);
    if (myself->interrupted == false)
    {
        sprintf(message, "SF\n");
    }
    else
    {
        sprintf(message, "BS\n");
    }
    if (tcp_send(myself->childrenfd[myself->nofchildren], message, strlen(message), debug))
    {
        return -1;
    }

    return 0;
}
/* say stream is broken */
int stream_broke(peer_conneciton * myself, bool debug)
{
    /*variablres*/
    char bmessage[SSBUFFSIZE];
    int i = 0;
    /* say stream is broken */
    if (sprintf(bmessage, "BS\n")<0)
    {
        perror("[ERROR] Failed to formulate broken stream message ");
        return -1;
    }
    for (i = 0; i < myself->nofchildren; i++)
    {
        if (tcp_send(myself->childrenfd[i], bmessage, strlen(bmessage), debug))
        {
            return -1;
        }
    }
    return 0;
}

/* redirect message */
int stream_redirect(int tempchild, char * ipaddrtport, bool debug)
{
    char message[SBUFFSIZE];

    if (sprintf(message, "RE %s\n", ipaddrtport)<0)
    {
        perror("[ERROR] Failed to formulate redirect message ");
        return -1;
    }
    if (tcp_send(tempchild, message, strlen(message), debug))
    {
        return -1;
    }

    return 0;
}
