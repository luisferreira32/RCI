/* includes of external libs */

/* own header include */
#include "GUI.h"

/* functions */

void render_header(void)
{
    printf("********************************************************\n");
    printf("**          WELCOME TO THE STREAM LISTENNER!          **\n");
    printf("**                                                    **\n");
    printf("** Notice input is case sensitive and from this list: **\n");
    printf("** streams                                            **\n");
    printf("** status                                             **\n");
    printf("** display on/off                                     **\n");
    printf("** format ascii/hex                                   **\n");
    printf("** debug on/off                                       **\n");
    printf("** tree                                               **\n");
    printf("** exit                                               **\n");
    printf("** HELP (extra command)                               **\n");
    printf("********************************************************\n");
}

void render_insert(void)
{
    printf(">> ");
}

void render_help(void)
{
    printf("YOU HAVE ASKED FOR HELP! GOOD LUCK\n");
}
