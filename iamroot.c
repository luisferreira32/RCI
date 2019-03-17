/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

/* the user includes */
#include "startup.h"
#include "UI.h"
#include "GUI.h"
#include "root_api.h"
#include "structs.h"

/* our main */

int main(int argc, char const *argv[])
{
    /* auxiliary variables delcaration*/
    char request_buffer[SBUFFSIZE], answer_buffer[MBUFFSIZE];
    int quit = 0, selected = 0;
    struct timeval root_timer, *timerp = NULL;
    /* main variables */
    iamroot_connection my_connect;
    client_interface my_ci;
    peer_conneciton myself;
    fd_set rfds;

    /* first set the strcut to it's values */
    if(set_connection(&my_connect, &my_ci, &myself, argc, argv))
    {
        printf("[LOG] Application will terminate\n");
        return -1;
    }
    FD_ZERO(&rfds);

    /* connect to stream since there is a stream ID */
    if(sprintf(request_buffer,"WHOISROOT %s:%s:%d %s:%d\n", my_connect.streamname, my_connect.streamip, my_connect.streamport, my_connect.ipaddr, my_connect.uport)<0)
    {
        perror("[ERROR] Formulating stream request failed ");
        return -1;
    }
    /* based on request to root server */
    if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, my_ci.debug))
    {
        printf("[LOG] Running request failed\n");
        return -1;
    }
    if(process_answer(answer_buffer, &my_connect, &myself, my_ci.debug))
    {
        printf("[LOG] Processing answer failed\n");
        return -1;
    }

    /* MAIN LOOP USING SELECT WITH TIMER TO REFRESH */
    render_header();
    while (quit == 0)
    {
        /* reset every loop */
        /* root specific -> timer & access server */
        if(myself.amiroot == true)
        {
            root_timer.tv_usec = 0;
            root_timer.tv_sec = my_connect.tsecs;
            timerp = &root_timer;
            FD_SET(myself.accessfd, &rfds);
        }
        else
        {
            timerp = NULL;
        }
        /* all other file descritors*/
        FD_SET(STDIN, &rfds);

        /* NUMBER OF FD IS myself.nofchildren + 1 (father) + 1(stdin) + 1(access) !!!!*/
        if((selected = select(1,&rfds, NULL, NULL, timerp))< 0)
        {
            perror("[ERROR] Select failed ");
            break;
        }

        if(selected == 0)
        {
            if(refresh_root(&my_connect, my_ci.debug) < 0)
            {
                printf("[ERROR] Failed to refresh root ownership\n");
                break;
            }
            continue;
        }
        else
        {
            /* if the file read is stdin */
            if(FD_ISSET(STDIN, &rfds))
            {
                if(fgets(request_buffer, SBUFFSIZE, stdin) == NULL)
                {
                    break;
                }
                quit = read_command(request_buffer, &my_connect, &my_ci);
            }

            /* read access fd if root*/
            if(myself.amiroot == true && FD_ISSET(myself.accessfd, &rfds))
            {
                /* POPs are set on TCP connections while being added to the tree
                this function can fail and mantain the good functioning of the stream */
                pop_reply(&my_connect, myself.accessfd, myself.ipaddrtport, my_ci.debug);
            }

            /* run through all POSSIBLE file descriptors that are selected*/
        }
    }
    /* use SELECT*/
    /* if STDIN has on buffer, FGETS */
    /* if UDP i'm root so do peer connection */
    /* if TCP recieve and propagate & print if display is on*/
    /* when timed out send the focking message to ROOT and re-start*/

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
        if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, my_ci.debug))
        {
            printf("[ERROR] Error on running remove request\n");
            return -1;
        }
    }
    else
    {
        printf("[LOG] Good bye father...\n");
    }

    /* warn peers of disconnecting .. !*/

    return 0;
}
