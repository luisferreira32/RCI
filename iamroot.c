/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* the user includes */
#include "startup.h"
#include "UI.h"
#include "root_api.h"
#include "structs.h"

/* our main */

int main(int argc, char const *argv[])
{
    /* auxiliary variables delcaration*/
    char request_buffer[SBUFFSIZE], answer_buffer[MBUFFSIZE];
    /* main variables */
    iamroot_connection my_connect;
    client_interface my_ci;
    peer_conneciton myself;

    /* first set the strcut to it's values */
    set_myselfdefault(&myself);
    if(set_connection(&my_connect, &my_ci, argc, argv))
    {
        printf("[LOG] Application will terminate\n");
        return -1;
    }

    /* connect to stream since there is a stream ID */
    if(sprintf(request_buffer,"WHOISROOT %s:%s:%d %s:%d\n", my_connect.streamname, my_connect.streamip, my_connect.streamport, my_connect.ipaddr, my_connect.uport)<0)
    {
        perror("[ERROR] Formulating stream request failed ");
        return -1;
    }
    /* based on request to root server */
    if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, &my_ci))
    {
        printf("[ERROR] Running request failed\n");
        return -1;
    }
    if(process_answer(answer_buffer, &my_connect, &myself))
    {
        printf("[ERROR] Processing answer failed\n");
        return -1;
    }

    /* show UI */
    if(show_UI(&my_connect, &my_ci))
    {
        printf("[LOG] Application will terminate\n");
        return -1;
    }

    /* disconnecting procedures */
    if(myself.amiroot == true)
    {
        /* take stream root off root server */
        if(sprintf(request_buffer,"REMOVE %s:%s:%d\n", my_connect.streamname, my_connect.streamip, my_connect.streamport)<0)
        {
            perror("[ERROR] Formulating stream request failed ");
            return -1;
        }
        /* run request */
        if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, &my_ci))
        {
            printf("[ERROR] Error on running remove request\n");
            return -1;
        }
    }
    else
    {
        printf("Good bye father...\n");
    }

    /* warn peers of disconnecting .. !*/

    return 0;
}
