/* includes of external libs */

/* own header include */
#include "UI.h"

/* functions */

/* transforms a string in lowercase letters */
void string_lowercase(char * string)
{
    int i = 0;
    for (i = 0; i < (int)strlen(string); i++)
    {
        string[i] = (string[i] <= 'Z' && string[i] >= 'A') ? string[i] + 32 : string[i];
    }
}

/* interprets the user commands */
int read_command(char * command_buffer, iamroot_connection * my_connect, client_interface * my_ci, peer_conneciton * myself)
{
    /*variables*/
    char answer_buffer[MBUFFSIZE];
    int i =0;

    string_lowercase(command_buffer);
    /* command intrepertation */
    if(strcmp(command_buffer, "streams\n") == 0)
    {
        if(run_request("DUMP\n", answer_buffer, MBUFFSIZE, my_connect, my_ci->debug))
        {
            printf("[LOG] Error on running dump request\n");
        }
        if(process_answer(answer_buffer, NULL, NULL, my_ci->debug))
        {
            printf("[LOG] Error processing dump answer\n");
        }
    }
    else if(strcmp(command_buffer, "status\n") == 0)
    {
        printf("\nSTATUS\n\n");
        printf("streamID: %s\n", my_connect->streamID);
        printf("interrupted: %s\n",  myself->interrupted ? "yes":"no");
        printf("amiroot: %s\n", myself->amiroot ? "yes":"no" );
        if (myself->amiroot)
        {
            printf("access server: %s:%d\n", my_connect->ipaddr, my_connect->uport);
        }
        else
        {
            printf("father id: %s:%d\n", my_connect->streamip, my_connect->streamport );
        }
        printf("tcp access point: %s:%d\n",my_connect->ipaddr, my_connect->tport );
        printf("tcpsessions/occupied: %d/%d\n", my_connect->tcpsessions, myself->nofchildren);
        for (i = 0; i < myself->nofchildren; i++)
        {
            printf("child %d: %s\n",i+1,myself->childrenaddr[i]);
        }
        printf("\n");
    }
    else if(strcmp(command_buffer, "display on\n") == 0)
    {
        my_ci->display = true;
        printf("[LOG] Command processed \n");
    }
    else if(strcmp(command_buffer, "display off\n") == 0)
    {
        my_ci->display = false;
        printf("[LOG] Command processed \n");
    }
    else if(strcmp(command_buffer, "format ascii\n") == 0)
    {
        strcpy(my_ci->format, "ascii");
        printf("[LOG] Command processed \n");
    }
    else if(strcmp(command_buffer, "format hex\n") == 0)
    {
        strcpy(my_ci->format, "hex");
        printf("[LOG] Command processed \n");
    }
    else if(strcmp(command_buffer, "debug on\n") == 0)
    {
        my_ci->debug = true;
        printf("[LOG] Command processed \n");
    }
    else if(strcmp(command_buffer, "debug off\n") == 0)
    {
        my_ci->debug = false;
        printf("[LOG] Command processed \n");
    }
    else if(strcmp(command_buffer, "tree\n") == 0)
    {
        if (myself->amiroot == false)
        {
            printf("[LOG] You are not root to tree\n");
        }
        else
        {
            /* make tree request, when recieving tree msg auto print it?*/
            myself->treeprinter = myself->nofchildren;
            printf("\nTREE\n\n");
            printf("%s\n", my_connect->streamID);
            printf("%s:%d %d (",my_connect->ipaddr, my_connect->tport, my_connect->tcpsessions );
            for (i = 0; i < myself->nofchildren; i++)
            {
                printf(" %s ", myself->childrenaddr[i]);
            }
            printf(")\n");
            stream_treequery(myself,my_ci->debug);
        }

    }
    else if(strcmp(command_buffer, "exit\n") == 0)
    {
        return 1;
    }
    else if(strcmp(command_buffer, "help\n") == 0)
    {
        render_help();
    }
    else if(strcmp(command_buffer, "clear\n") == 0)
    {
        printf("\x1B[1;1H\x1B[2J");
        fflush(stdout);
        return 0;
    }
    else
    {
        printf("Invalid command, write HELP if needed.\n");
        return 0;
    }

    return 0;
}
