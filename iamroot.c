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
    char request_buffer[SBUFFSIZE], answer_buffer[MBUFFSIZE], recv_buffer[SBUFFSIZE];
    int nfds = 0, i = 0, j = 0, noftries = 0, connected_child = 0, buff_end = 0, buff_end2 = 0;
    int extra = 0, extrachild[SSBUFFSIZE] = {0};
    struct timeval root_timer, *timerp = NULL;
    /* flags */
    int quit = 0, connected = 0, selected = 0, querying = 1;
    /* main variables */
    pop_list *head = NULL, *iter = NULL;
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
        /* open access to tcp connections */
        if((myself.recvfd = receive_listeners(my_connect.tport))<0)
        {
            printf("[LOG] Failed to open tcp socket \n");
            quit = 1;
        }
    }

    /* MAIN LOOP USING SELECT WITH TIMER TO REFRESH */
    while (quit == 0)
    {
        /* connects IF not already connected */
        if(connected == 0)
        {
            sleep(1); /* to make sure changes are made on ROOT server */
            printf("[LOG] Connecting to a stream atempt %d...\n", noftries+1);
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
                noftries++;
                continue;
            }
            else if(quit == 1)
            {
                /* if not root and it failed to connect exit */
                break;
            }
            else
            {
                /* it's operational go for it!*/
                connected = 1; noftries = 0; myself.interrupted = false;
                stream_status(&myself, my_ci.debug);
                render_header();
            }
        }

        /* reset every loop */
        FD_ZERO(&rfds);
        /* when tcp disconnect kernel KEEPS fd open for three minutes.. or program termination */
        nfds = 10 + my_connect.tcpsessions;
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
            FD_SET(myself.childrenfd[i], &rfds);nfds++;
        }

        /* check for own pops if ROOT */
        if (myself.amiroot == true && myself.popcounter < my_connect.bestpops)
        {
            /* check if I have pop, otherwise request for pops only if needed */
            if (my_connect.tcpsessions > myself.nofchildren)
            {
                if(sprintf(myself.popaddr[myself.popcounter], "%s:%d", my_connect.ipaddr, my_connect.tport) <0)
                {
                    perror("[ERROR] Setting owned POPs ");
                }
                else
                {
                    myself.popcounter++;
                    querying = 0; /* no need to wait if i can accept */
                }
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
                continue;
            }

            /* read the stream and propagate to children */
            if (FD_ISSET(myself.fatherfd,&rfds))
            {
                /* if size received is 0 it's a closing statement, reconnect */
                memset(recv_buffer, 0, SBUFFSIZE);
                if((connected = stream_recv(myself.fatherfd, recv_buffer, my_ci.debug))==0)
                {
                    tcp_disconnect(myself.fatherfd);myself.interrupted = true;
                    stream_status(&myself, my_ci.debug);
                }
                else if(connected < 0)
                {
                    quit = 1;
                }
                else
                {
                    /* check for line endings */
                    buff_end = 0; buff_end2 = strlen(myself.fatherbuff);
                    while ((int)strlen(recv_buffer) > buff_end )
                    {
                        if ( recv_buffer[buff_end] != '\n' && buff_end2< MBUFFSIZE)
                        {
                            myself.fatherbuff[buff_end2] = recv_buffer[buff_end];
                            buff_end2++;
                        }
                        else
                        {
                            myself.fatherbuff[buff_end2] = recv_buffer[buff_end];
                            if ((extra = stream_recv_downstream(myself.fatherbuff, &myself, &my_connect, &my_ci, extra, &head))<0)
                            {
                                printf("[LOG] Failed to treat father's message\n");
                                extra = 0;
                            }
                            buff_end2 = 0;
                            memset(myself.fatherbuff, 0, MBUFFSIZE);
                        }
                        buff_end ++;
                    }
                }
            }

            /* check if it's a peer trying to join the tree */
            if (FD_ISSET(myself.recvfd, &rfds))
            {
                /* accept it and send a message of welcome or redirect */
                if (myself.nofchildren < my_connect.tcpsessions)
                {
                    /* accept to a new fd */
                    if ((myself.childrenfd[myself.nofchildren] = accept_children(myself.recvfd)) < 0)
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
                /* else connect, redirect to first child and disconnect */
                else
                {
                    if ((i = accept_children(myself.recvfd)) < 0)
                    {
                        printf("[LOG] Error accepting children \n");
                    }
                    if (myself.nofchildren <= 0)
                    {
                        printf("[LOG] No living soul to redirect, goodbye.\n");

                    }
                    else
                    {
                        if (stream_redirect(i, myself.childrenaddr[myself.nofchildren-1], my_ci.debug))
                        {
                            printf("[LOG] Failed to redirect temp child \n");
                        }
                    }
                    tcp_disconnect(i);
                }

            }

            /* run through all POSSIBLE file descriptors that are selected*/
            for (i = 0; i < myself.nofchildren; i++)
            {
                if (FD_ISSET(myself.childrenfd[i], &rfds))
                {
                    /* if size received is 0 it's a closing statement, reconnect */
                    memset(recv_buffer, 0, SBUFFSIZE);
                    if((connected_child=stream_recv(myself.childrenfd[i], recv_buffer, my_ci.debug))==0)
                    {
                        tcp_disconnect(myself.childrenfd[i]);
                        for (j = i; j < myself.nofchildren-1; j++)
                        {
                            myself.childrenfd[j] = myself.childrenfd[j+1];
                        }
                        myself.nofchildren--;
                        break;
                    }
                    else if(connected_child < 0)
                    {
                        quit = 1;
                    }
                    else
                    {
                        /* check for line endings */
                        buff_end = 0; buff_end2 = strlen(myself.childbuff[i]);
                        while ((int)strlen(recv_buffer) > buff_end )
                        {
                            if ( recv_buffer[buff_end] != '\n' && buff_end2 < MBUFFSIZE)
                            {
                                myself.childbuff[i][buff_end2] = recv_buffer[buff_end];
                                buff_end2++;
                            }
                            else
                            {
                                myself.childbuff[i][buff_end2] = recv_buffer[buff_end];
                                if ((extrachild[i] = stream_recv_upstream(i, myself.childbuff[i], &myself, &my_connect, my_ci.debug, extrachild[i], &head, &querying))<0)
                                {
                                    printf("[LOG] Failed to treat child's message\n");
                                    extrachild[i] = 0;
                                }
                                buff_end2= 0;
                                memset(myself.childbuff[i], 0, MBUFFSIZE);
                            }
                            buff_end ++;
                        }
                    }
                }
            }


            /* read access fd if root*/
            if(myself.amiroot == true && FD_ISSET(myself.accessfd, &rfds))
            {
                /* reply if we finished querying current tree */
                if(querying == 0)
                {
                    myself.popcounter--;
                    if (myself.popcounter < 0)
                    {
                        printf("[LOG] Unexpected lack of pops...\n");
                        quit = 1;break;
                    }
                    pop_reply(&my_connect, myself.accessfd, myself.popaddr[myself.popcounter], my_ci.debug);
                    querying = 1;
                }
                /* otherwise better get some NEW pops, the old ones may be corrupted */
                else
                {
                    querying = 1;myself.popcounter = 0;
                    if (stream_popquery(&myself, &my_connect, my_ci.debug))
                    {
                        printf("[LOG] Failed to send pop queries \n");
                        quit = 1;break;
                    }
                    /* and wait for pop replies will you? */
                    sleep(1);
                }
            }

        }/* select close */

    }/* main loop close */

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
        printf("[LOG] Good bye root...\n");
    }


    /* close all open tcps */
    if (myself.accessfd != -1)tcp_disconnect(myself.accessfd);
    if(myself.fatherfd != -1)tcp_disconnect(myself.fatherfd);
    for ( i = 0; i < myself.nofchildren; i++)tcp_disconnect(myself.childrenfd[i]);
    if(myself.recvfd != -1)tcp_disconnect(myself.recvfd);

    /* free any alocated memory */
    free_memory(&myself, &my_connect);
    iter = head;
    while (head != NULL)
    {
        iter = head;
        head = head->next;
        free(iter);
    }

    return 0;
}
