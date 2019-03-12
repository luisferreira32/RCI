/* includes of external libs */

/* own header include */
#include "UI.h"

/* functions */

/* main function of the UI */
int show_UI(root_connection * my_connect, client_interface * my_ci)
{
    /* variables */
    int quit = 0;
    char command_buffer[20];

    /* our user interface calling protocols */
    render_header();

    while (!quit)
    {
        render_insert();
        if(scanf("%s",command_buffer) < 0)
        {
            perror("[LOG] Failed to read command ");
        }
        quit = read_command(command_buffer, my_ci);
    }

    return 0;
}

/* interprets the user commands */
int read_command(char * command_buffer, client_interface * my_ci)
{
    if(strcmp(command_buffer, "streams") == 0)
    {

    }
    else if(strcmp(command_buffer, "status") == 0)
    {

    }
    else if(strcmp(command_buffer, "display on") == 0)
    {
        my_ci->display = true;
    }
    else if(strcmp(command_buffer, "display off") == 0)
    {
        my_ci->display = false;
    }
    else if(strcmp(command_buffer, "format ascii") == 0)
    {
        strcpy(my_ci->format, "ascii");
    }
    else if(strcmp(command_buffer, "format hex") == 0)
    {
        strcpy(my_ci->format, "hex");
    }
    else if(strcmp(command_buffer, "debug on") == 0)
    {
        my_ci->debug = true;
    }
    else if(strcmp(command_buffer, "debug off") == 0)
    {
        my_ci->debug = false;
    }
    else if(strcmp(command_buffer, "tree") == 0)
    {

    }
    else if(strcmp(command_buffer, "exit") == 0)
    {
        return 1;
    }
    else if(strcmp(command_buffer, "HELP") == 0)
    {

    }
    else
    {

    }

    return 0;
}
