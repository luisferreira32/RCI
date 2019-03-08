/* includes of external libs */

/* own header include */
#include "startup.h"

/* functions */

/* sets the default value for connection */
void set_default(root_connection * my_connect)
{
    strcpy(my_connect->streamname,"\0");
}

/* check if a text is an IP */
int is_ip(char * text)
{
    struct sockaddr_in test;
    return inet_pton(AF_INET, text, &(test.sin_addr));
}

/* sets the connection struct according to arguments */
int set_connection(root_connection * my_connect, int argc, const char ** argv)
{
    /* declaration */
    int i = 1;

    /* set default */
    set_default(my_connect);

    /* read if first argument is a stream ID address */
    if(argv[i][0] != 45)
    {
        /* read arguments of streamid */
        if(sscanf(argv[i], "%[^:]:%[^:]:%d", my_connect->streamname, my_connect->streamip, &my_connect->streamport) < 0)
        {
            perror("[LOG] Streamid reading ");
        }
        /* check if it's valid */
        if(!is_ip(my_connect->streamip))
        {
            printf("[LOG] Check IP in streamid\n" );
            printf("%s\n", my_connect->streamip );
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
          i++;
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
          /* treat it */
          i++;
        }
        else if (strcmp(argv[i], "-u") == 0)
        {
          /* treat it */
          i++;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
          /* treat it */
          i++;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
          /* treat it */
          i++;
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
          /* treat it */
          i++;
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
          /* treat it */
          i++;
        }
        /* options WITHOUT arguments*/
        else if (strcmp(argv[i], "-b") == 0)
        {
          /* treat it */
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
          /* treat it */
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i],"--help") == 0)
        {
          /* treat it */
          return -1;
        }
        /* more else if clauses */
        else /* default: */
        {
            printf("\n%s is an invalid command\n-h or --help  option for command list\n\n", argv[i]);
        }
    }

    return 0;
}
