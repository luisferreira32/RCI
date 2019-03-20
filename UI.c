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
int read_command(char * command_buffer, iamroot_connection * my_connect, client_interface * my_ci)
{
    /*variables*/
    char answer_buffer[MBUFFSIZE];

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
        printf("%s\n", my_connect->streamID);
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
    else if(strcmp(command_buffer, "help\n") == 0)
    {
        render_help();
    }
    else
    {
        printf("Invalid command, write HELP if needed.\n");
        return 0;
    }
    printf("[LOG] Command processed \n");

    return 0;
}
