/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

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
    int nfds = 0, i = 0, j = 0, noftries = 0;
    struct timeval root_timer, *timerp = NULL;
    /* flags */
    int quit = 0, connected = 0, selected = 0, accessing = 0;
    /* main variables */
    iamroot_connection my_connect;
    client_interface my_ci;
    peer_conneciton myself;
    fd_set rfds;

    /* first set the strcut to it's values */
    if(set_connection(&my_connect, &my_ci, &myself, argc, argv))
    {
        printf("[LOG] Failed the start up\n");
        return 0;
    }

    /* initial procedures */
    if (quit == 0)
    {
        quit = set_memory(&myself, &my_connect);
    }

    /* MAIN LOOP USING SELECT WITH TIMER TO REFRESH */
    while (quit == 0)
    {
        /* connects IF not already connected */
        if(connected == 0)
        {
            sleep(1); /* to make sure changes are made on ROOT server */
            printf("[LOG] Connecting to a stream...\n");
            /* connect to stream since there is a stream ID */
            if(sprintf(request_buffer,"WHOISROOT %s %s:%d\n", my_connect.streamID, my_connect.ipaddr, my_connect.uport)<0)
            {
                perror("[ERROR] Formulating stream request failed ");
                quit = 1;
            }
            /* based on request to root server connect to stream OR peer */
            if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, my_ci.debug))
            {
                printf("[LOG] Running request failed\n");
                quit = 1;
            }

            if(quit == 1 || process_answer(answer_buffer, &my_connect, &myself, my_ci.debug))
            {
                printf("[LOG] Processing answer failed\n");
                quit = 1;
            }
            /* if it was a root dc - try three times to connect */
            if (myself.amiroot == true && quit == 1 && noftries < 3)
            {
                connected = 0; quit = 0;
                stream_broke(&myself, my_ci.debug);
                noftries++;
            }
            /* if not root and is failed to connect exit */
            else if(quit == 1)
            {
                break;
            }
            else
            {
                /* it's operational go for it!*/
                connected = 1;
                render_header();
            }
        }

        /* reset every loop */
        FD_ZERO(&rfds);
        nfds = 3 + my_connect.tcpsessions;
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
        FD_SET(STDIN, &rfds);
        FD_SET(myself.fatherfd, &rfds);
        FD_SET(myself.recvfd, &rfds);
        for (i = 0; i < myself.nofchildren; i++)
        {
            FD_SET(myself.childrenfd[i], &rfds);
        }

        /* ask N best pops if ROOT */
        if (myself.amiroot == true && myself.popcounter < my_connect.bestpops)
        {
            /* send pop request message - the child will reply later */
            /* check if I have pop, otherwise request for pops */
            if (my_connect.tcpsessions > myself.nofchildren+accessing)
            {
                if(sprintf(myself.ipaddrtport[myself.popcounter], "%s:%d", my_connect.ipaddr, my_connect.tport) <0)
                {
                    perror("[ERROR] Setting owned POPs ");
                }
                myself.popcounter++;
            }
        }

        /* OUR IMPORTANT SELECT TO CHECK INPUTS */
        if((selected = select(nfds,&rfds, NULL, NULL, timerp))< 0)
        {
            perror("[ERROR] Select failed ");
            break;
        }
        /* when timed out, only root times out, refresh on root server*/
        if(selected == 0)
        {
            if(refresh_root(&my_connect, my_ci.debug) < 0)
            {
                printf("[ERROR] Failed to refresh root ownership\n");
                break;
            }
            continue;
        }
        /* else read all other possible file descriptors */
        else
        {
            /* if the file read is stdin */
            if(FD_ISSET(STDIN, &rfds))
            {
                if(fgets(request_buffer, SBUFFSIZE, stdin) == NULL)
                {
                    break;
                }
                quit = read_command(request_buffer, &my_connect, &my_ci, &myself);
            }

            /* read access fd if root*/
            if(myself.amiroot == true && FD_ISSET(myself.accessfd, &rfds))
            {
                myself.popcounter--;
                /* exception if we try to access the root WHEN it still has slots*/
                if (myself.nofchildren < my_connect.tcpsessions)
                {
                    accessing=1;
                }
                if (myself.popcounter < 0)
                {
                    printf("[LOG] Unexpected lack of POPs \n");
                    myself.popcounter = 0;
                }
                /* reply the access server request */
                pop_reply(&my_connect, myself.accessfd, myself.ipaddrtport[myself.popcounter], my_ci.debug);
            }

            /* read the stream and propagate to children */
            if (FD_ISSET(myself.fatherfd,&rfds))
            {
                /* if size recieved is 0 it's a closing statement, reconnect */
                if((connected = stream_recv_downstream(&myself, &my_ci, &my_connect))==0)
                {
                    tcp_disconnect(myself.fatherfd);
                }
                else if(connected < 0)
                {
                    quit = 1;
                }
            }

            /* check if it's a peer trying to join the tree */
            if (FD_ISSET(myself.recvfd, &rfds))
            {
                accessing=0;
                /* accept it and send a message of welcome or redirect */
                if (myself.nofchildren < my_connect.tcpsessions)
                {
                    /* accept to a new fd */
                    if ((myself.childrenfd[myself.nofchildren] = accept_children(myself.recvfd, myself.childrenaddr[myself.nofchildren])) < 0)
                    {
                        printf("[LOG] Error accepting children \n");
                    }
                    else
                    {
                        /* and send WElcome*/
                        if (stream_welcome(&my_connect, &myself, my_ci.debug))
                        {
                            printf("[LOG] Failed to welcome child\n");
                            tcp_disconnect(myself.childrenfd[myself.nofchildren]);
                            myself.nofchildren--;
                        }
                        myself.nofchildren++;
                    }
                }
                /* else connect, redirect and disconnect */
                else
                {
                    if ((i = accept_children(myself.recvfd, NULL)) < 0)
                    {
                        printf("[LOG] Error accepting children \n");
                    }
                    myself.popcounter--;
                    if (myself.popcounter < 0 )
                    {
                        printf("[LOG] Unexpected lack of POPs \n");
                        myself.popcounter = 0;
                    }
                    if (stream_redirect(i, myself.ipaddrtport[myself.popcounter], my_ci.debug))
                    {
                        printf("[LOG] Failed to redirect temp child \n");
                    }
                    tcp_disconnect(i);
                }

            }

            /* run through all POSSIBLE file descriptors that are selected*/
            for (i = 0; i < myself.nofchildren; i++)
            {
                if (FD_ISSET(myself.childrenfd[i], &rfds))
                {
                    /* check the upstream message if it's 0 - disconnect child */
                    if (stream_recv_upstream(myself.childrenfd[i], &myself, &my_connect, my_ci.debug)==0)
                    {
                        tcp_disconnect(myself.childrenfd[i]);
                        for (j = i; j < myself.nofchildren-1; j++)
                        {
                            myself.childrenfd[i] = myself.childrenfd[i+1];
                        }
                        myself.nofchildren--;
                    }
                    /* else go on! */
                }
            }
        }
    }

    /* disconnecting procedures */
    if(myself.amiroot == true)
    {
        /* take stream root off root server */
        if(sprintf(request_buffer,"REMOVE %s\n", my_connect.streamID)<0)
        {
            perror("[ERROR] Formulating stream request failed ");
        }
        /* run request */
        if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, my_ci.debug))
        {
            printf("[LOG] Error on running remove request\n");
        }
    }
    else
    {
        printf("[LOG] Good bye father...\n");
    }


    /* close all open tcps */
    if (myself.accessfd != -1)tcp_disconnect(myself.accessfd);
    if(myself.fatherfd != -1)tcp_disconnect(myself.fatherfd);
    for ( i = 0; i < myself.nofchildren; i++)tcp_disconnect(myself.childrenfd[i]);
    if(myself.recvfd != -1)tcp_disconnect(myself.recvfd);

    /* free any alocated memory */
    free_memory(&myself, &my_connect);

    return 0;
}
