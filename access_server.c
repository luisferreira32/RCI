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

/* make a respond to the request */

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
        printf("[ERROR] Running request failed\n");
        return -1;
    }

    /* and expect an URROOT */
    if (sscanf(answer_buffer, "%s ", request_buffer)<0)
    {
        perror("[ERROR] Failed to get root answer");
        return -1;
    }
    if(strcmp(request_buffer, "URROOT") != 0)
    {
        printf("[LOG] Root was stolen!\n");
        return -2;
    }

    return 0;
}

/* non root request */
int pop_request(char * answer_buffer, size_t buffer_size, char * asaddr, int asport, bool debug)
{
    /* variables */
    int socketfd = -1;
    char buff[BBUFFSIZE],request[SSBUFFSIZE];
    struct sockaddr_in peer;

    /* create socket for client type */
    if((socketfd = udp_create_client(NULL, asaddr, asport, &peer))< 0)
    {
        perror("[ERROR] Failed to create socket to access server ");
        return -1;
    }
    /* create message */
    if (sprintf(request, "POPREQ\n")<0)
    {
        perror("[ERROR] Formulating request ");
        return -1;
    }
    /* send check */
    if(udp_send(socketfd, request, strlen(request), &peer,debug) < 0)
    {
        perror("[ERROR] Failed to send request to root ");
        return -1;
    }

    /* recieve check */
    if(udp_recv(socketfd, buff, BBUFFSIZE, &peer, debug) < 0 )
    {
        perror("[ERROR] Failed to recv answer from access server");
        return -1;
    }
    if(buffer_size < strlen(buff))
    {
        printf("[ERROR] Access server overflowed buffer \n");
        return -1;
    }

    /* save the answer */
    strcpy(answer_buffer, buff);
    udp_destroy(socketfd);

    return 0;
}
