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
#include "stream_api.h"

/* our main */

int main(int argc, char const *argv[])
{
    /* auxiliary variables delcaration*/
    char request_buffer[SBUFFSIZE], answer_buffer[MBUFFSIZE];
    int quit = 0, connected = 0, selected = 0, nfds = 0, i = 0;
    struct timeval root_timer, *timerp = NULL;
    /* main variables */
    iamroot_connection my_connect;
    client_interface my_ci;
    peer_conneciton myself;
    fd_set rfds;

    /* first set the strcut to it's values */
    if(set_connection(&my_connect, &my_ci, &myself, argc, argv))
    {
        printf("[LOG] Failed the start up\n");
        quit = 1;
    }

    /* allocate memory accordingly and open service to recieve children*/
    myself.childrenfd = (int *)malloc(sizeof(int)*my_connect.tcpsessions);
    if(quit == 0 && (myself.recvfd = recieve_listeners(my_connect.tport))<0)
    {
        printf("[LOG] Failed to open tcp socket \n");
        quit = 1;
    }

    /* MAIN LOOP USING SELECT WITH TIMER TO REFRESH */
    while (quit == 0)
    {
        /* connects IF not already connected */
        if(connected == 0)
        {
            printf("[LOG] Connecting to a stream ...\n");
            /* connect to stream since there is a stream ID */
            if(sprintf(request_buffer,"WHOISROOT %s:%s:%d %s:%d\n", my_connect.streamname, my_connect.streamip, my_connect.streamport, my_connect.ipaddr, my_connect.uport)<0)
            {
                perror("[ERROR] Formulating stream request failed ");
                quit = 1;break;
            }
            /* based on request to root server connect to stream OR peer */
            if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, my_ci.debug))
            {
                printf("[LOG] Running request failed\n");
                quit = 1;break;
            }
            if(process_answer(answer_buffer, &my_connect, &myself, my_ci.debug))
            {
                printf("[LOG] Processing answer failed\n");
                quit = 1;break;
            }
            /* it's operational go for it!*/
            connected = 1;
            render_header();
        }
        /* reset every loop */
        FD_ZERO(&rfds);nfds = 2;
        /* root specific -> timer & access server */
        if(myself.amiroot == true)
        {
            root_timer.tv_usec = 0;
            root_timer.tv_sec = my_connect.tsecs;
            timerp = &root_timer;
            FD_SET(myself.accessfd, &rfds);nfds++;
        }
        else
        {
            timerp = NULL;
        }
        /* all other file descritors*/
        FD_SET(STDIN, &rfds);nfds++;
        FD_SET(myself.fatherfd, &rfds);nfds++;
        FD_SET(myself.recvfd, &rfds);nfds++;
        for (i = 0; i < myself.nofchildren; i++)
        {
            FD_SET(myself.childrenfd[i], &rfds); nfds++;
        }

        /* NUMBER OF FD IS myself.nofchildren + 1 (father) + 1(stdin) + 1(access) !!!!*/
        if((selected = select(nfds,&rfds, NULL, NULL, timerp))< 0)
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

            /* read the stream and propagate to children */
            if (FD_ISSET(myself.fatherfd,&rfds))
            {
                /* if size recieved is 0 it's a closing statement, reconnect */
                connected = stream_recv(&myself, &my_ci);
            }

            /* check if it's a peer trying to join the tree */
            if (FD_ISSET(myself.recvfd, &rfds))
            {
                /* accept it and send a message of welcome or redirect */
            }

            /* run through all POSSIBLE file descriptors that are selected*/
            for (i = 0; i < myself.nofchildren; i++)
            {
                if (FD_ISSET(myself.childrenfd[i], &rfds))
                {
                    /* check the upstream message  */
                }
            }
        }
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

    /* close all open tcps */
    if (myself.accessfd != -1 && myself.fatherfd != -1 && myself.recvfd != -1)
    {
        tcp_disconnect(myself.accessfd);
        tcp_disconnect(myself.fatherfd);
        tcp_disconnect(myself.recvfd);
        for ( i = 0; i < myself.nofchildren; i++)
        {
            tcp_disconnect(myself.childrenfd[i]);
        }
    }

    /* free any alocated memory */
    free(myself.childrenfd);

    return 0;
}
