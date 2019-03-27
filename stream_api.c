/* own header include */
#include "stream_api.h"

/* functions definitions */

/* basic functions inside the package
    - open port and connect to father
    - open service and create relation with childrenfd
    - read from fd and depending on origin & message propagate /display*/

/* create listening socket */
int receive_listeners(int accessport)
{
    return tcp_server(accessport);
}

/* create a fd for a child */
int accept_children(int recvfd)
{
    return  tcp_accept(recvfd, NULL);
}

/* connect to stream and return its file descritor*/
int connect_stream(char * streamip, int streamport)
{
    return tcp_client(NULL, streamip, streamport);
}

/* flush to buffer */
int stream_recv(int sockfd, char * smallbuffer, bool debug)
{
    int size_recv = -1;
    size_recv = tcp_recv(sockfd, smallbuffer, SBUFFSIZE, debug);
    return size_recv;
}

/* read stream capsule downstream and return values accondringly */
int stream_recv_downstream(char * capsule, peer_conneciton* myself, iamroot_connection * my_connect, client_interface * my_ci, int extra, pop_list ** head)
{
    /* variables */
    char header[SSBUFFSIZE], size[5], message[SBUFFSIZE], queryID[5];
    int data_size = 0, i = 0, bestpops = 0;
    pop_list  * new, *iter;

    /* if i'm root it can only be DATA, let's capsule it and resend*/
    if (myself->amiroot == true)
    {
        return stream_data(capsule, myself, my_ci);
    }
    /* if we're recieving an extra it means what is on capsule is raw DATA */
    if (extra > 0)
    {
        extra -= strlen(capsule);
        if (stream_data(capsule, myself, my_ci))
        {
            return -1;
        }
        return extra;
    }

    /* check according to header the procedure */
    memset(header, 0, SSBUFFSIZE);
    if (sscanf(capsule, "%2s ", header)!=1)
    {
        perror("[ERROR] Failed to fetch header from stream message ");
    }

    /* depending on header treat it  */
    /* can only be data if the stream is not broken */
    if (strcmp("DA", header) == 0 && myself->interrupted == false)
    {
        /*read the data*/
        if (sscanf(capsule, "%s %[^\n]", header, size) != 2)
        {
            printf("[LOG] Failed to fetch data \n");
            return -1;
        }
        data_size = strtol(size, NULL, 16);
        return data_size;
    }
    /* a welcome message after joining the stream */
    else if(strcmp(header, "WE") == 0)
    {
        memset(capsule, 0, SBUFFSIZE);
        if (sprintf(capsule, "NP %s:%d\n", my_connect->ipaddr, my_connect->tport)<0)
        {
            perror("[ERROR] Failed to formulate NP message ");
        }
        if (tcp_send(myself->fatherfd, capsule, strlen(capsule), my_ci->debug))
        {
            printf("[LOG] Failed to reply to WELCOME message\n");
        }
    }
    /* a redirect message after joining a full spot*/
    else if(strcmp(header, "RE")==0)
    {
        if (sscanf(capsule, "%s %[^:]:%d", header, my_connect->streamip, &(my_connect->streamport)) != 3)
        {
            perror("[ERROR] Redirect message read ");
        }
        /* disconnect from previous and reconnect */
        tcp_disconnect(myself->fatherfd);
        myself->fatherfd = -1;

        if ((myself->fatherfd = connect_stream(my_connect->streamip, my_connect->streamport)) <0)
        {
            printf("[LOG] Failed to connect to new stream source\n");
            return -1;
        }
    }
    /* check if stream is broken or not */
    else if(strcmp(header, "SF")==0)
    {
        if (myself->interrupted == true)
        {
            myself->interrupted = false;
            for (i = 0; i < myself->nofchildren; i++)
            {
                if (tcp_send(myself->childrenfd[i], capsule, strlen(capsule), my_ci->debug))
                {
                    return -1;
                }
            }
        }
    }
    else if(strcmp(header, "BS")==0)
    {
        if (myself->interrupted == false )
        {
            myself->interrupted = true;
            for (i = 0; i < myself->nofchildren; i++)
            {
                if (tcp_send(myself->childrenfd[i], capsule, strlen(capsule), my_ci->debug))
                {
                    return -1;
                }
            }
        }
    }
    else if(strcmp(header, "PQ") == 0)
    {
        /*read the query ID */
        if (sscanf(capsule, "%s %s ", header, queryID) != 2)
        {
            printf("[LOG] PQ read failed \n");
            return -1;
        }
        /* recieving a pop query check if it's duplicate*/
        iter = *head;
        while (iter != NULL && strcmp(iter->queryID, queryID) != 0)
        {
            iter = iter->next;
        }
        /* if it's a new pq, add it to the list, else use the previous */
        if (iter == NULL)
        {
            /* add an element to our pop list */
            new = (pop_list *)malloc(sizeof(pop_list));
            new->next = NULL;
            add_list_element(head,new);
        }
        else
        {
            new = iter;
        }
        /* and retrieve PQ id and best pops*/
        if (sscanf(capsule, "%s %s %d\n", header, new->queryID, &(new->bestpops)) != 3)
        {
            printf("[LOG] Failed to get PQ ID & bestpops\n");
            free_list_element(head,new);
            return -1;
        }
        /* if i can still accept tcps connections send my POPs*/
        if (myself->nofchildren < my_connect->tcpsessions)
        {
            memset(message, 0, SBUFFSIZE);
            bestpops = (my_connect->tcpsessions)-(myself->nofchildren);
            if (sprintf(message, "PR %s %s:%d %d\n", new->queryID, my_connect->ipaddr, my_connect->tport, bestpops)<0)
            {
                perror("[ERROR] Failed to formulate welcome message ");
                free_list_element(head,new);
                return -1;
            }
            if (tcp_send(myself->fatherfd, message, strlen(message), my_ci->debug))
            {
                free_list_element(head,new);
                return -1;
            }
            new->bestpops--;
        }
        /* if he still wants more pops request from children*/
        if (new->bestpops > 0)
        {
            memset(message, 0, SBUFFSIZE);
            if (sprintf(message, "PQ %s %d\n", new->queryID, new->bestpops)<0)
            {
                perror("[ERROR] Failed to formulate pop query message ");
                free_list_element(head,new);
                return -1;
            }
            for (i = 0; i < myself->nofchildren; i++)
            {
                if (tcp_send(myself->childrenfd[i], message, strlen(message), my_ci->debug))
                {
                    return -1;
                }
            }
        }
        else
        {
            free_list_element(head,new);
        }
    }
    else if (strcmp(header,"TQ")==0)
    {
        /* if we receive from father a TQ is to propagate... */
        if (stream_treequery(myself, my_ci->debug) || stream_treereply(myself, my_connect, my_ci->debug))
        {
            printf("[LOG] Failed to proccess tree query\n");
        }
    }
    else
    {
        printf("[LOG] Protocol not followed by father\n");
    }

    return 0;
}

/* IF it's a data we'll print it and resend it */
int stream_data(char * data, peer_conneciton * myself, client_interface * my_ci)
{
    int i = 0;
    char * capsule = (char *)malloc(sizeof(char)*strlen(data)+10);

    if (sprintf(capsule, "DA %04X\n%s", (unsigned int)strlen(data), data)<0)
    {
        perror("[ERROR] Failed to encapsule ");
    }

    /* put a \n at the end of the capsule (if original data was SBUFFSIZE)*/
    if (capsule[strlen(capsule)-1] != '\n')
    {
        strcat(capsule, "\n");
    }

    /*display*/
    if (my_ci->display == true)
    {
        /* take off the \n that is per default on data*/
        if (data[strlen(data)-1]=='\n')
        {
            data[strlen(data)-1]='\0';
        }

        /*ascii*/
        if (strcmp(my_ci->format, "ascii")==0)
        {
            printf("%s\n", data);
        }
        else /*or hex*/
        {
            for (i = 0; i < (int)strlen(data); i++)
            {
                printf("%02X", (unsigned int)data[i]);
            }
            printf("\n");
        }
    }
    /* then resend it to children*/
    for (i = 0; i < myself->nofchildren; i++)
    {
        if (tcp_send(myself->childrenfd[i], capsule, strlen(capsule),my_ci->debug))
        {
            printf("[LOG] Failed to propagate message to child %d\n", i+1);
        }
    }

    free(capsule);
    return 0;
}


/* upstream message treatment */
int stream_recv_upstream(int origin, char * capsule, peer_conneciton* myself, iamroot_connection * my_connect, bool debug, int extra, pop_list ** head)
{
    /* variables */
    char  header[SSBUFFSIZE], queryID[4], smallbuffer[SBUFFSIZE], mediumbuffer1[MBUFFSIZE], mediumbuffer2[MBUFFSIZE];
    pop_list *iter;

    /* if there is an extra flag it means we're recieving the rest of a tree reply*/
    if (extra > 0)
    {
        /* two \n mean end of TQ */
        if (capsule[strlen(capsule)-1] == '\n' && capsule[strlen(capsule)-2]== '\n')
        {
            /* it's finnally all on the capsule */
            if (myself->treeprinter > 0)
            {
                memset(mediumbuffer1, 0, MBUFFSIZE);memset(smallbuffer, 0, SBUFFSIZE);
                sscanf(capsule, "%s %[^\n] %499c",header, smallbuffer, mediumbuffer1);
                printf("%s (", smallbuffer);
                while (mediumbuffer1[0] != '\0' && mediumbuffer1[0] != '\n' )
                {
                    /* READ EACH NEW LINE AND PRINT IT */
                    strcpy(mediumbuffer2, mediumbuffer1);
                    memset(mediumbuffer1, 0, MBUFFSIZE); memset(smallbuffer, 0, SBUFFSIZE);
                    sscanf(mediumbuffer2, "%[^\n] %499c", smallbuffer, mediumbuffer1);
                    printf(" %s ", smallbuffer);
                    /* and say you'll have to print for N childs that gonna TR you */
                    myself->treeprinter++;
                }
                printf(")\n");
                /* finished printing one of the TRs*/
                myself->treeprinter--;
            }
            /* if i'm not tree printer i should resend it now above */
            else
            {
                if (tcp_send(myself->fatherfd, capsule, strlen(capsule), debug))
                {
                    return -1;
                }
            }
            /* reached end of TR message return 0 as extra */
            return 0;
        }
        /* still not in the end of the TR message */
        return 1;
    }

    /* check according to header the procedure */
    memset(header, 0, SSBUFFSIZE);
    if (sscanf(capsule, "%2s ", header) ==0)
    {
        perror("[ERROR] Failed to fetch header from stream message ");
        return -1;
    }

    /* if it's a new pop i must save it for possible redirect*/
    if (strcmp(header, "NP")==0)
    {
        if (sscanf(capsule, "%s %s\n", header, myself->childrenaddr[origin])!=2)
        {
            printf("[LOG] Did not add NP\n" );
        }
    }
    else if (strcmp(header, "PR") == 0)
    {
        /* if i'm root just save the POP if we have slot*/
        if (myself->amiroot == true && my_connect->bestpops > myself->popcounter)
        {
            if (sscanf(capsule, "%s %s %s ", header, queryID, myself->popaddr[myself->popcounter]) != 3)
            {
                printf("[LOG] Failed to get pop \n");
            }
            else
            {
                myself->popcounter++;
            }
        }
        /* or propagate according to the requests */
        else if(myself->amiroot == false)
        {
            if (sscanf(capsule, "%s %s ",header, queryID) != 2)
            {
                printf("[LOG] Failed to get popq ID\n");
                return -1;
            }
            iter = *head;
            /* check which request it corresponded */
            while (iter != NULL)
            {
                /*only reply if we have the queryID asking */
                if (strcmp(queryID, iter->queryID)==0)
                {
                    if (tcp_send(myself->fatherfd, capsule, strlen(capsule), debug))
                    {
                        return -1;
                    }
                    iter->bestpops--;
                    break;
                }
                iter = iter->next;
            }
            /* if there was no request discard message */
            /* else take out request of list if no longer needed */
            if (iter != NULL && iter->bestpops <= 0)
            {
                free_list_element(head,iter);
            }
        }
        /* being root WITHOUT bestpops, means discard */
    }
    else if (strcmp(header, "TR") == 0)
    {
        /* read how many more newlines we'll need to see */
        if (sscanf(capsule, "%s %s %d",header, smallbuffer, &extra) != 3)
        {
            printf("[LOG] Failed to get tr addr \n");
            return -1;
        }
        /* start to recieve the extra, when we finish we send it upstream */
        return 1;
    }
    else
    {
        printf("[LOG] Child not following Protocol\n" );
    }

    return 0;

}

/* welcoming message */
int stream_welcome(iamroot_connection * my_connect, peer_conneciton * myself, bool debug)
{
    char message[SBUFFSIZE];

    /* welcome and stream info */
    if (sprintf(message, "WE %s\n", my_connect->streamID)<0)
    {
        perror("[ERROR] Failed to formulate welcome message ");
        return -1;
    }
    if (tcp_send(myself->childrenfd[myself->nofchildren], message, strlen(message), debug))
    {
        return -1;
    }
    /*strea status*/
    memset(message,0,SBUFFSIZE);
    if (myself->interrupted == false)
    {
        sprintf(message, "SF\n");
    }
    else
    {
        sprintf(message, "BS\n");
    }
    if (tcp_send(myself->childrenfd[myself->nofchildren], message, strlen(message), debug))
    {
        return -1;
    }

    return 0;
}
/* say stream is broken */
int stream_status(peer_conneciton * myself, bool debug)
{
    /*variablres*/
    char bmessage[SSBUFFSIZE];
    int i = 0;
    /* say stream is broken or not*/
    if (myself->interrupted == false)
    {
        sprintf(bmessage, "SF\n");
    }
    else
    {
        sprintf(bmessage, "BS\n");
    }
    /* to all childs */
    for (i = 0; i < myself->nofchildren; i++)
    {
        if (tcp_send(myself->childrenfd[i], bmessage, strlen(bmessage), debug))
        {
            return -1;
        }
    }
    return 0;
}

/* redirect message */
int stream_redirect(int tempchild, char * ipaddrtport, bool debug)
{
    char message[SBUFFSIZE];

    if (sprintf(message, "RE %s\n", ipaddrtport)<0)
    {
        perror("[ERROR] Failed to formulate redirect message ");
        return -1;
    }
    if (tcp_send(tempchild, message, strlen(message), debug))
    {
        return -1;
    }

    return 0;
}

/* query all my children for POPs and receive the first pops */
int stream_popquery(peer_conneciton * myself, iamroot_connection * my_connect, bool debug)
{
    int i = 0;
    char message[SBUFFSIZE];

    for (i = 0; i < myself->nofchildren; i++)
    {
        memset(message, 0, SBUFFSIZE);
        if (sprintf(message, "PQ %04X %d\n", i ,my_connect->bestpops-myself->popcounter)<0)
        {
            perror("[ERROR] Failed to formulate pq message ");
            return -1;
        }
        if (tcp_send(myself->childrenfd[i], message, strlen(message), debug))
        {
            return -1;
        }
    }
    return 0;
}

/* query all childs for tree struct */
int stream_treequery(peer_conneciton * myself, bool debug)
{
    int i = 0;
    char message[SBUFFSIZE];

    for (i = 0; i < myself->nofchildren; i++)
    {
        memset(message, 0, SBUFFSIZE);
        if (sprintf(message, "TQ %s\n", myself->childrenaddr[i])<0)
        {
            perror("[ERROR] Failed to formulate tq message ");
            return -1;
        }
        if (tcp_send(myself->childrenfd[i], message, strlen(message), debug))
        {
            return -1;
        }
    }
    return 0;
}

/* to dispatch the reply to father */
int stream_treereply(peer_conneciton * myself,iamroot_connection * my_connect, bool debug)
{
    int i = 0, error = 0;
    char smallbuffer[60];
    char * treemsg = (char *)malloc(sizeof(char)*60*myself->nofchildren + 60*sizeof(char));

    if (sprintf(treemsg, "TR %s:%d %d\n", my_connect->ipaddr, my_connect->tport, my_connect->tcpsessions)<0)
    {
        perror("[ERROR] Failed to formulate tq message ");error = -1;
    }
    for (i = 0; i < myself->nofchildren; i++)
    {
        if (sprintf(smallbuffer, "%s\n", myself->childrenaddr[i])<0)
        {
            perror("[ERROR] Failed to formulate tq message ");error = -1;
        }
        if (error != 0)break;
        strcat(treemsg, smallbuffer);
    }
    strcat(treemsg, "\n\0");

    if (error == 0 && tcp_send(myself->fatherfd, treemsg, strlen(treemsg), debug))
    {
        printf("[LOG] Failed to send TR \n");error = -1;
    }

    free(treemsg);
    return error;
}
