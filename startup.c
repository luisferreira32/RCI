/* includes of external libs */

/* own header include */
#include "startup.h"

/* functions */

/* sets the default value for connection */
void set_default(iamroot_connection * my_connect, client_interface * my_ci, peer_conneciton * myself)
{
    strcpy(my_connect->streamID, "\0");
    strcpy(my_connect->streamname,"\0");
    strcpy(my_connect->streamip, "\0");
    my_connect->streamport = -1;

    strcpy(my_connect->ipaddr, "127.0.0.1");

    my_connect->tport = DTCPPORT;
    my_connect->uport = DUDPPORT;

    strcpy(my_connect->rsaddr, DROOTIP);
    my_connect->rsport = DUDPROOTPORT;
    my_connect->tcpsessions = DTCPSESSIONS;
    my_connect->bestpops = DBESTPOPS;
    my_connect->tsecs = DTSECS;

    my_ci->display = true;
    my_ci->debug = false;

    strcpy(my_ci->format, DFORMAT);

    myself->amiroot = true;
    myself->interrupted = false;
    myself->ipaddrtport = NULL;
    myself->popcounter = 0;
    myself->accessfd = -1;
    myself->recvfd = -1;
    myself->fatherfd = -1;
    myself->childrenfd = NULL;
    myself->childrenaddr = NULL;
    myself->nofchildren = 0;

}


/* check if a text is an IP */
int is_ip(char * text)
{
    struct sockaddr_in test;
    return inet_pton(AF_INET, text, &(test.sin_addr));
}

/* sets the connection struct according to arguments */
int set_connection(iamroot_connection * my_connect, client_interface * my_ci, peer_conneciton * myself, int argc, const char ** argv)
{
    /* declaration */
    int i = 1;
    char answer_buffer[MBUFFSIZE];

    /* set default */
    set_default(my_connect, my_ci, myself);

    /* if cast with no stream ID proceed as instructed */
    if ( argc < 2 || (argv[i][0] == '-' && argv[i][1] != 'h'))
    {
        if(run_request("DUMP\n", answer_buffer, MBUFFSIZE, my_connect, my_ci->debug))
        {
            printf("[LOG] Error on running request\n");
            return -1;
        }
        if(process_answer(answer_buffer, NULL, NULL, my_ci->debug))
        {
            printf("[LOG] Error processing answer\n");
        }
        return -1;
    }

    /* read if first argument is a stream ID address */
    if(argv[i][0] != 45)
    {
        /* read arguments of streamid */
        if(sscanf(argv[i], "%[^:]:%[^:]:%d", my_connect->streamname, my_connect->streamip, &my_connect->streamport) == 0)
        {
            perror("[ERROR] Streamid reading ");
            return -1;
        }
        if (sscanf(argv[i], "%s", my_connect->streamID) == 0)
        {
            perror("[ERROR] Streamid reading ");
            return -1;
        }
        /* check if it's valid */
        if(!is_ip(my_connect->streamip) || my_connect->streamport < 1025)
        {
            printf("[LOG] Check IP & port in streamid\n" );
            printf("%s:%d\n", my_connect->streamip, my_connect->streamport );
            return -1;
        }
        i++;
    }

    /* read arguments, if any */
    for (i = i; i < argc; i++)
    {
        /* options with arguments option */
        if (strcmp(argv[i], "-i") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -i requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -i requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i],"%s",my_connect->ipaddr) == 0)
            {
                perror("[ERROR] Reading interface IP");
                return -1;
            }
            /* validate */
            if(!is_ip(my_connect->ipaddr))
            {
                printf("[LOG] Check IP in interface address -i\n" );
                printf("%s\n", my_connect->ipaddr );
                return -1;
            }
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -t requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -t requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i],"%d",&my_connect->tport)==0)
            {
                perror("[ERROR] Reading port");
                return -1;
            }
            /* small check */
            if(my_connect->tport < 1025)
            {
                printf("[LOG] Check port in option -t\n" );
                printf("%d\n", my_connect->tport);
                return -1;
            }
        }
        else if (strcmp(argv[i], "-u") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -u requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -u requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i],"%d",&my_connect->uport)==0)
            {
                perror("[ERROR] Reading port");
            }
            /* small check */
            if(my_connect->uport < 1025)
            {
                printf("[LOG] Check port in option -u\n" );
                printf("%d\n", my_connect->uport);
                return -1;
            }
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -s requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -s requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i], "%[^:]:%d", my_connect->rsaddr, &my_connect->rsport) == 0)
            {
                perror("[ERROR] Root server reading ");
                return -1;
            }
            /* validate */
            if(!is_ip(my_connect->rsaddr) || my_connect->rsport < 1025)
            {
                printf("[LOG] Check IP and port of root server -r\n" );
                printf("ip: %s, port: %d\n", my_connect->rsaddr, my_connect->rsport );
                return -1;
            }
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -p requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -p requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i],"%d",&my_connect->tcpsessions)==0)
            {
                perror("[ERROR] Reading tcpsessions");
            }
            /* small check */
            if(my_connect->tcpsessions < 1)
            {
                printf("[LOG] Check number of tcp sessions (>0) -p\n" );
                printf("%d\n", my_connect->tcpsessions);
                return -1;
            }
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -n requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -n requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i],"%d",&my_connect->bestpops)==0)
            {
                perror("[ERROR] Reading bestpops");
            }
            /* small check */
            if(my_connect->bestpops < 1)
            {
                printf("[LOG] Check number of bestpops (>0) -x\n" );
                printf("%d\n", my_connect->bestpops);
                return -1;
            }
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            /* treat it */
            /* check errors */
            if(++i == argc)
            {
                printf("[LOG] -x requires argument\n");
                return -1;
            }
            if(argv[i][0] == 45)
            {
                printf("[LOG] -x requires argument\n");
                return -1;
            }
            /* read them */
            if(sscanf(argv[i],"%d",&my_connect->tsecs)==0)
            {
                perror("[ERROR] Reading tsecs");
            }
            /* small check */
            if(my_connect->tsecs < 1)
            {
                printf("[LOG] Check the root record refresh timer (>0) -x\n" );
                printf("%d\n", my_connect->tsecs);
                return -1;
            }
        }
        /* options WITHOUT arguments*/
        else if (strcmp(argv[i], "-b") == 0)
        {
          /* treat it */
          my_ci->display = false;
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
          /* treat it */
          my_ci->debug = true;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i],"--help") == 0)
        {
          /* treat it */
          display_help();
          /* ends right after print */
          return -1;
        }
        /* more else if clauses */
        else /* default: */
        {
            printf("\n[ERROR] \"%s\" is an invalid command\n-h or --help  option for command list\n\n", argv[i]);
            return -1;
        }
    }

    return 0;
}

/* display help options */
void display_help(void)
{
    printf("\niamroot [<streamID>] [-i <ipaddr>] [-t <tport>] [-u <uport>]\n");
    printf("        [-s <rsaddr>[:<rsport>]] [-p <tcpsessions>] \n");
    printf("        [-n <bestpops>] [-x <tsecs>][-b] [-d] [-h]\n\n");

    printf("<streamID> is the stream identification with syntax <streamname>:<IP>:<port>\n");
    printf("-i <ipaddr> is the interface IP address\n" );
    printf("-t <tport> is the tcp port to accept connections of peers \n");
    printf("-u <uport> is the udp port for access server\n");
    printf("-s <rsaddr>[:<rsport>] is the root server ip and port \n");
    printf("-p <tcpsessions> is the number of tcp sessions this root is able to accept\n");
    printf("-n <bestpops> is the number of access points to retrieve in connection attempt\n");
    printf("-x <tsecs> is the number of seconds to refresh root server info\n");
    printf("-b option to disable byte stream \n");
    printf("-d option to enable debug mode \n");
    printf("-h to see this log again \n\n");
}
