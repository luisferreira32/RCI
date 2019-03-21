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
    int quit = 0, connected = 0, selected = 0, nfds = 0, i = 0, j = 0;
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
        return 0;
    }

    /* initial procedures */
    if (quit == 0)
    {
        /* allocate memory accordingly and open service to recieve children*/
        myself.childrenfd = (int *)malloc(sizeof(int)*my_connect.tcpsessions);
        if((myself.recvfd = recieve_listeners(my_connect.tport))<0)
        {
            printf("[LOG] Failed to open tcp socket \n");
            quit = 1;
        }
        /* allocate for POPs and set them default*/
        if (myself.amiroot == true)
        {
            myself.ipaddrtport = (char **)malloc(sizeof(char *)*my_connect.bestpops);
            for (i = 0; i < my_connect.bestpops; i++)
            {
                myself.ipaddrtport[i] = (char *)malloc(sizeof(char )*SBUFFSIZE);
            }
        }
    }

    /* MAIN LOOP USING SELECT WITH TIMER TO REFRESH */
    while (quit == 0)
    {
        /* connects IF not already connected */
        if(connected == 0)
        {
            printf("[LOG] Connecting to a stream ...\n");
            /* connect to stream since there is a stream ID */
            if(sprintf(request_buffer,"WHOISROOT %s %s:%d\n", my_connect.streamID, my_connect.ipaddr, my_connect.uport)<0)
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

        /* ask N best pops if ROOT */
        if (myself.amiroot == true && myself.popcounter < my_connect.bestpops)
        {
            /* check if I have pop, otherwise request for pops */
            if (my_connect.tcpsessions > myself.nofchildren)
            {
                if(sprintf(myself.ipaddrtport[myself.popcounter], "%s:%d", my_connect.ipaddr, my_connect.tport) <0)
                {
                    perror("[ERROR] Setting owned POPs ");
                }
                myself.popcounter++;
            }
            else
            {
                /* ASK FOR POPS */
            }
        }

        /* OUR IMPORTANT SELECT TO CHECK INPUTS */
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
                myself.popcounter--;
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
                connected = stream_recv_downstream(&myself, &my_ci, &my_connect);
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
                /* else connect, redirect and disconnect */
                else
                {
                    if ((i = accept_children(myself.recvfd)) < 0)
                    {
                        printf("[LOG] Error accepting children \n");
                    }
                    myself.popcounter--;
                    if (myself.popcounter < 0 || stream_redirect(i, myself.ipaddrtport[myself.popcounter], my_ci.debug))
                    {
                        myself.popcounter = 0;
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
        if(sprintf(request_buffer,"REMOVE %s:%s:%d\n", my_connect.streamname, my_connect.streamip, my_connect.streamport)<0)
        {
            perror("[ERROR] Formulating stream request failed ");
        }
        /* run request */
        if(run_request(request_buffer, answer_buffer, MBUFFSIZE, &my_connect, my_ci.debug))
        {
            printf("[LOG] Error on running remove request\n");
        }
        /* de allocate root specific memory */
        for (i = 0; i < my_connect.bestpops; i++)
        {
            free(myself.ipaddrtport[i]);
        }
        free(myself.ipaddrtport);
    }
    else
    {
        printf("[LOG] Good bye father...\n");
    }


    /* close all open tcps */
    if (myself.accessfd != -1)tcp_disconnect(myself.accessfd);
    if(myself.fatherfd != -1)tcp_disconnect(myself.fatherfd);
    if(myself.recvfd != -1)tcp_disconnect(myself.recvfd);
    for ( i = 0; i < myself.nofchildren; i++)tcp_disconnect(myself.childrenfd[i]);

    /* free any alocated memory */
    free(myself.childrenfd);

    return 0;
}
