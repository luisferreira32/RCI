/* own header include */
#include "root_api.h"

/* GENERAL FUNCTIONS */

/* creates udp, sends udp, recieves udp, closes udp */
int run_request(char * request, char * answer_buffer, size_t buffer_size, iamroot_connection * my_connect, client_interface * my_ci)
{
    /* variables */
    int socketfd = -1;
    char buff[1000];
    struct sockaddr_in peer;

    /* create socket for client type */
    if((socketfd = udp_create_client(NULL, my_connect->rsaddr, my_connect->rsport, &peer))< 0)
    {
        perror("[LOG] Failed to create socket to root ");
        return -1;
    }
    /* send check */
    if(udp_send(socketfd, request, strlen(request), &peer,my_ci->debug) < 0)
    {
        perror("[LOG] Failed to send request to root ");
        return -1;
    }
    /* and exit IF we're removing*/
    if(sscanf(request,"%s ",buff)<0)
    {
        perror("[LOG] sscanf failed ");
        return -1;
    }
    if(strcmp(buff,"REMOVE")==0)
    {
        return 0;
    }
    /* recieve check */
    if(udp_recv(socketfd, buff, 1000, &peer, my_ci->debug) < 0 )
    {
        perror("[LOG] Failed to recv answer from root");
        return -1;
    }
    if(buffer_size < strlen(buff))
    {
        printf("[LOG] Buffer overflowed \n");
        return -1;
    }

    /* save the answer */
    strcpy(answer_buffer, buff);
    udp_destroy(socketfd);

    return 0;
}

/* takes the awnser buff and understands stuff */
int process_answer(char * answer, peer_conneciton * myself)
{
    /*variables*/
    char first[10];
    char buff[500];

    /* get the keyword */
    if(sscanf(answer,"%s %s", first, buff) == 0)
    {
        printf("[LOG] Error reading root server reply\n");
    }
    /* compare with possible replies */
    if(strcmp(first,"URROOT") == 0)
    {
        /* open access server and connect to stream */

    }
    else if(strcmp(first,"ROOTIS") == 0)
    {
        /* do request on access server  */
    }
    else if(strcmp(first, "STREAMS") == 0)
    {
        /* prints log */
        printf("\n%s", answer);
    }
    else if(strcmp(first, "ERROR") == 0)
    {
        /* prints log */
        printf("\n%s", answer);
        return -1;
    }
    else
    {
        printf("[LOG] Invalid request %s\n",first);
        return -1;
    }

    return 0;
}
