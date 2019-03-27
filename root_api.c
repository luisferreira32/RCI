/* own header include */
#include "root_api.h"

/* GENERAL FUNCTIONS */

/**********************************************************/
/* name: run_request
** description: run a request to the root server and wait
for answer, notice that within udp api we have a timer for
receive so there is no problem if root server crashes */
int run_request(char * request, char * answer_buffer, size_t buffer_size, iamroot_connection * my_connect, bool debug)
{
    /* variables */
    int socketfd = -1;
    char buff[BBUFFSIZE];
    struct sockaddr_in peer;

    /* create socket for client type */
    if((socketfd = udp_create_client(NULL, my_connect->rsaddr, my_connect->rsport, &peer))< 0)
    {
        printf("[LOG] Failed to create socket to root ");
        return -1;
    }
    /* send check */
    if(udp_send(socketfd, request, strlen(request), &peer,debug) < 0)
    {
        perror("[ERROR] Failed to send request to root ");
        return -1;
    }
    /* and exit IF we're removing*/
    if(sscanf(request,"%s ",buff)==0)
    {
        perror("[ERROR] sscanf of answer failed ");
        return -1;
    }
    if(strcmp(buff,"REMOVE")==0)
    {
        udp_destroy(socketfd);
        return 0;
    }
    /* receive check */
    if(udp_recv(socketfd, buff, BBUFFSIZE, &peer, debug) < 0 )
    {
        perror("[ERROR] Failed to recv answer from root");
        return -1;
    }
    if(buffer_size < strlen(buff))
    {
        printf("[LOG] Root answer buffer overflowed \n");
    }

    /* save the answer */
    memset(answer_buffer, 0, buffer_size);
    strcpy(answer_buffer, buff);
    udp_destroy(socketfd);

    return 0;
}

/**********************************************************/
/* name: process_answer
** description: taking the root server answer act accordingly */
int process_answer(char * answer, iamroot_connection * my_connect, peer_conneciton * myself, bool debug)
{
    /*variables*/
    char first[10], buff[BBUFFSIZE-10], streamID[SBUFFSIZE], asaddr[SBUFFSIZE];
    int asport = -1;

    /* get the keyword */
    if(sscanf(answer,"%s %[^\n]", first, buff) == 0)
    {
        perror("[ERROR] Error reading root server reply ");
    }
    /* compare with possible replies */
    if(strcmp(first,"URROOT") == 0)
    {
        /* open access server and connect to stream */
        myself->amiroot = true;
        myself->interrupted = false;
        if(myself->accessfd == -1 && open_access_server(my_connect->uport, myself))
        {
            printf("[LOG] Failed to open access server \n");
            return -1;
        }
        /* reset the streamip and port source, now from root! */
        if(sscanf(my_connect->streamID, "%[^:]:%[^:]:%d", my_connect->streamname, my_connect->streamip, &my_connect->streamport) != 3)
        {
            perror("[ERROR] Streamid reading ");
            return -1;
        }
        /* connect to stream */
        if ((myself->fatherfd = connect_stream(my_connect->streamip, my_connect->streamport)) <0)
        {
            printf("[LOG] Failed to connect to stream source\n");
            return -1;
        }
    }
    else if(strcmp(first,"ROOTIS") == 0)
    {
        /* do request on access server  */
        myself->amiroot = false;
        /* first get the access server */
        if (sscanf(buff, "%s %[^:]:%d", streamID, asaddr, &asport)==0)
        {
            perror("[ERROR] Failed to get access server IP & port ");
            return -1;
        }

        /* verify stuff */
        if(strcmp(streamID, my_connect->streamID) != 0)
        {
            printf("[LOG] Wrong stream to connect \n");
            return -1;
        }
        /* notice that streamip and streamport WILL be changed for stream point*/
        if(pop_request(my_connect,asaddr, asport, debug ) < 0)
        {
            printf("[LOG] Failed to request POP on access server\n");
            return -1;
        }

        /* connect to peer as stream source */
        if ((myself->fatherfd = connect_stream(my_connect->streamip, my_connect->streamport)) <0)
        {
            printf("[LOG] Failed to connect to stream source\n");
            return -1;
        }

    }
    else if(strcmp(first, "STREAMS") == 0)
    {
        /* prints log */
        printf("\n%s", answer);
    }
    else if(strcmp(first, "ERROR") == 0)
    {
        /* prints log */
        printf("\n[ROOT] %s\n", answer);
        return -1;
    }
    else
    {
        printf("[LOG] Invalid request %s\n",first);
        return -1;
    }

    return 0;
}
