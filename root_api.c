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
