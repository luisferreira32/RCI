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

/**********************************************************/
/* name: main
** description:
The main operates in three phases:
initialization - sets all variables with arguments and memory
main loop - manages application functioning
finalization - frees memory and closes openned tcp sessions

In the main loop we can see three distinct parts:
connecting - tries to connect to a stream source or exits
initializing - treats variables that need values every loop
select - treats all types of incoming connections */
int main(int argc, char const *argv[])
{
    /* auxiliary variables delcaration*/
    char request_buffer[SBUFFSIZE], answer_buffer[MBUFFSIZE], recv_buffer[SBUFFSIZE];
    int nfds = 0, i = 0, j = 0, noftries = 0, connected_child = 0, buff_end = 0, buff_end2 = 0;
    int extra = 0, *extrachild = NULL;
    struct timeval timer;
    /* flags */
    int quit = 0, connected = 0, selected = 0;
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
        set_memory(&myself, &my_connect);
        extrachild = (int *)malloc(sizeof(int)*my_connect.tcpsessions);
        for (i = 0; i < my_connect.tcpsessions; i++)extrachild[i]=0;
        /* open access to tcp connections */
        if((myself.recvfd = receive_listeners(my_connect.tport))<0)
        {
            printf("[LOG] Failed to open tcp socket \n");
            quit = 1;
        }
        timer.tv_usec = 0;
        timer.tv_sec = my_connect.tsecs;
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
            /* try three times to connect */
            if (quit == 1 && noftries < 3)
            {
                connected = 0; quit = 0;
                noftries++;
                continue;
            }
            else if(quit == 1)
            {
                /* if it failed to connect exit */
                break;
            }
            else
            {
                /* it's operational go for it!*/
                connected = 1; noftries = 0;
                stream_status(&myself, my_ci.debug);
                render_header();
            }
        }

        /* reset every loop */
        FD_ZERO(&rfds);
        /* when tcp disconnect kernel KEEPS fd open for three minutes.. or program termination */
        nfds = 2;
        /* root specific*/
        if(myself.amiroot == true)
        {
            FD_SET(myself.accessfd, &rfds);
            if(myself.accessfd>=nfds)nfds=myself.accessfd+1;
        }
        /* set all other file descritors*/
        FD_SET(STDIN, &rfds);
        FD_SET(myself.fatherfd, &rfds);
        if(myself.fatherfd>=nfds)nfds=myself.fatherfd+1;
        FD_SET(myself.recvfd, &rfds);
        if(myself.recvfd>=nfds)nfds=myself.recvfd+1;
        for (i = 0; i < myself.nofchildren; i++)
        {
            FD_SET(myself.childrenfd[i], &rfds);
            if(myself.childrenfd[i]>=nfds)nfds=myself.childrenfd[i]+1;
        }

        /* check for pops if ROOT */
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
                }
            }

            /* and POP query only when half of pops have been depleted */
            if (my_connect.bestpops/2 + 1 >  myself.popcounter)
            {
                if (stream_popquery(&myself, &my_connect, my_ci.debug))
                {
                    printf("[LOG] Failed to send pop queries \n");
                    quit = 1;break;
                }
                /* and wait for pop replies will you? */
                sleep(1);
            }
        }

        /* OUR IMPORTANT SELECT TO CHECK INPUTS */
        if((selected = select(nfds,&rfds, NULL, NULL, &timer))< 0)
        {
            perror("[ERROR] Select failed ");
            break;
        }

        /* when timed out*/
        if(selected == 0)
        {
            /* if i'm root refresh on server */
            if(myself.amiroot == true && refresh_root(&my_connect, my_ci.debug) < 0)
            {
                printf("[ERROR] Failed to refresh root ownership\n");
                break;
            }
            /* if i'm not root check for BS and exit if so*/
            if (myself.interrupted == true)
            {
                tcp_disconnect(myself.fatherfd);
                connected = 0;
            }

            timer.tv_usec = 0;
            timer.tv_sec = my_connect.tsecs;
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
                    printf("[LOG] Error on father tcp connection \n");
                    quit = 1;
                }
                else
                {
                    /* check for line endings */
                    buff_end = 0; buff_end2 = strlen(myself.fatherbuff);
                    while ((int)strlen(recv_buffer) > buff_end )
                    {
                        if ( recv_buffer[buff_end] != '\n' && buff_end2< MBUFFSIZE-1)
                        {
                            myself.fatherbuff[buff_end2] = recv_buffer[buff_end];
                            buff_end2++;
                        }
                        else
                        {
                            myself.fatherbuff[buff_end2] = recv_buffer[buff_end];
                            buff_end2++;
                            if ( recv_buffer[buff_end] != '\n')
                                myself.fatherbuff[buff_end2] = '\n';
                            /* treat message */
                            if ((extra = stream_recv_downstream(myself.fatherbuff, &myself, &my_connect, &my_ci, extra, &head))<0)
                            {
                                printf("[LOG] Failed to treat father's message\n");
                                quit = 1;break;
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
                        if (stream_welcome(&my_connect, &myself, my_ci.debug, myself.nofchildren))
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
                            if (myself.childrenaddr[0] != '\0')
                            {
                                strcpy(myself.childrenaddr[j],myself.childrenaddr[j+1]);
                            }
                        }
                        myself.nofchildren--;
                        break;
                    }
                    else if(connected_child < 0)
                    {
                        quit = 1;
                        printf("[LOG] Error on child tcp connection \n");
                        break;
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
                                buff_end2++;
                                if ((extrachild[i] = stream_recv_upstream(i, myself.childbuff[i], &myself, &my_connect, my_ci.debug, extrachild[i], &head))<0)
                                {
                                    printf("[LOG] Failed to treat child's message\n");
                                    extrachild[i] = 0;
                                }
                                /* if it's a TR wait for the rest of the message to treat it, else empty*/
                                if (extrachild[i] == 0)
                                {
                                    buff_end2= 0;
                                    memset(myself.childbuff[i], 0, MBUFFSIZE);
                                }
                                else if(buff_end2 > MBUFFSIZE-2)
                                {
                                    printf("[LOG] Middle buffer overflow\n");
                                    buff_end2 = 0;
                                    memset(myself.childbuff[i], 0, MBUFFSIZE);
                                }
                            }
                            buff_end ++;
                        }
                    }
                }
            }


            /* read access fd if root*/
            if(myself.amiroot == true && FD_ISSET(myself.accessfd, &rfds))
            {
                /* reply with pops  */
                myself.popcounter--;
                if (myself.popcounter < 0)
                {
                    printf("[LOG] Unexpected lack of pops...\n");
                    myself.popcounter = 0;
                    sleep(1);
                }
                else
                {
                    pop_reply(&my_connect, myself.accessfd, myself.popaddr[myself.popcounter], my_ci.debug);
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
    if(extrachild != NULL)free(extrachild);
    while (head != NULL)
    {
        iter = head;
        head = head->next;
        free(iter);
    }

    return 0;
}
