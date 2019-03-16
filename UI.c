/* includes of external libs */

/* own header include */
#include "UI.h"

/* functions */

/* interprets the user commands */
int read_command(char * command_buffer, iamroot_connection * my_connect, client_interface * my_ci)
{
    /*variables*/
    char answer_buffer[MBUFFSIZE];

    /* command intrepertation */
    if(strcmp(command_buffer, "streams\n") == 0)
    {
        if(run_request("DUMP\n", answer_buffer, MBUFFSIZE, my_connect, my_ci->debug))
        {
            printf("[ERROR] Error on running dump request\n");
        }
        if(process_answer(answer_buffer, NULL, NULL))
        {
            printf("[ERROR] Error processing dump answer\n");
        }
    }
    else if(strcmp(command_buffer, "status\n") == 0)
    {

    }
    else if(strcmp(command_buffer, "display on\n") == 0)
    {
        my_ci->display = true;
    }
    else if(strcmp(command_buffer, "display off\n") == 0)
    {
        my_ci->display = false;
    }
    else if(strcmp(command_buffer, "format ascii\n") == 0)
    {
        strcpy(my_ci->format, "ascii");
    }
    else if(strcmp(command_buffer, "format hex\n") == 0)
    {
        strcpy(my_ci->format, "hex");
    }
    else if(strcmp(command_buffer, "debug on\n") == 0)
    {
        my_ci->debug = true;
    }
    else if(strcmp(command_buffer, "debug off\n") == 0)
    {
        my_ci->debug = false;
    }
    else if(strcmp(command_buffer, "tree\n") == 0)
    {

    }
    else if(strcmp(command_buffer, "exit\n") == 0)
    {
        return 1;
    }
    else if(strcmp(command_buffer, "HELP\n") == 0)
    {
        render_help();
    }
    else
    {

    }

    return 0;
}
