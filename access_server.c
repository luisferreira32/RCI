/* includes of external libs */

/* own header include */
#include "access_server.h"

/* functions */

/* access server stuff */
int open_access_server(int port, peer_conneciton * myself)
{
    if((myself->accessfd = udp_server(port)) < 0)
    {
        printf("[ERROR] Failed to create file descriptor\n");
    }
    return 0;
}

/* register on root in tsecs function */
int refresh_root(iamroot_connection * my_connect, bool debug)
{
    /* variables */
    char request_buffer[SBUFFSIZE], answer_buffer[MBUFFSIZE];

    /* to refresh root server memory send a WHOISROOT*/
    if(sprintf(request_buffer,"WHOISROOT %s:%s:%d %s:%d\n", my_connect->streamname, my_connect->streamip, my_connect->streamport, my_connect->ipaddr, my_connect->uport)<0)
    {
        perror("[ERROR] Formulating stream request failed ");
        return -1;
    }
    /* based on request to root server */
    if(run_request(request_buffer, answer_buffer, MBUFFSIZE, my_connect, debug))
    {
        printf("[LOG] Running request failed\n");
        return -1;
    }

    /* and expect an URROOT */
    if (sscanf(answer_buffer, "%s ", request_buffer)==0)
    {
        perror("[ERROR] Failed to get root answer");
        return -1;
    }
    if(strcmp(request_buffer, "URROOT") != 0)
    {
        printf("[LOG] Root was stolen! %s\n", request_buffer);
        return -2;
    }

    return 0;
}

/* make a response to the request */
int pop_reply(iamroot_connection * my_connect, int accessfd, char * ipaddrtport, bool debug)
{
    /* variables */
    char answer[SBUFFSIZE],request[SSBUFFSIZE];
    struct sockaddr_in peer;

    /* recieve check */
    memset(request, 0, SSBUFFSIZE);
    if(udp_recv(accessfd, request, SSBUFFSIZE, &peer, debug) < 0 )
    {
        printf("[LOG] Failed to recv request on access server");
        return -1;
    }

    /* check if it's a true pop request*/
    if(strcmp(request, "POPREQ\n") != 0)
    {
        printf("[LOG] Unformated message on access socket: %s\n", request);
        return -1;
    }

    /* if it's a pop request answer with the pop*/
    if(sprintf(answer, "POPRESP %s %s\n", my_connect->streamID, ipaddrtport) <0)
    {
        perror("[ERROR] Failed to generate answer ");
        return -1;
    }
    if (udp_send(accessfd, answer, strlen(answer), &peer, debug)<0)
    {
        printf("[ERROR] Failed to send answer as access server \n");
        return -1;
    }

    return 0;
}

/* non root request */
int pop_request(iamroot_connection * my_connect, char * asaddr, int asport, bool debug)
{
    /* variables */
    int socketfd = -1;
    char request[SSBUFFSIZE], answer_buffer[MBUFFSIZE], streamID[SBUFFSIZE];
    struct sockaddr_in peer;

    /* create message */
    memset(request,0,SSBUFFSIZE);
    if (sprintf(request, "POPREQ\n")<0)
    {
        perror("[ERROR] Formulating request ");
        return -1;
    }
    /* create socket for client type */
    if((socketfd = udp_create_client(NULL, asaddr, asport, &peer))< 0)
    {
        printf("[LOG] Failed to create socket to access server\n");
        return -1;
    }
    /* send check */
    if(udp_send(socketfd, request, strlen(request), &peer,debug) < 0)
    {
        printf("[LOG] Failed to send request to root\n");
        udp_destroy(socketfd);
        return -1;
    }

    /* recieve check */
    if(udp_recv(socketfd, answer_buffer, MBUFFSIZE, &peer, debug) < 0 )
    {
        printf("[LOG] Failed to recv answer from access server\n");
        udp_destroy(socketfd);
        return -1;
    }
    udp_destroy(socketfd);

    /* and set the new streaming father */
    if(sscanf(answer_buffer, "%s %s %[^:]:%d\n", request, streamID, my_connect->streamip, &(my_connect->streamport)) <0)
    {
        perror("[ERROR] Failed to treat access server message");
        return -1;
    }

    /* last verify to protocol */
    if (strcmp(request, "POPRESP") != 0 || strcmp(streamID, my_connect->streamID) != 0)
    {
        printf("[ERROR] Access server not following protocol\n" );
        return -1;
    }

    return 0;
}
