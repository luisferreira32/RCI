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
int refresh_root(iamroot_connection * my_connect, client_interface * my_ci)
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
    if(run_request(request_buffer, answer_buffer, MBUFFSIZE, my_connect, my_ci))
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
