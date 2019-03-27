/* includes of external libs */

/* own header include */
#include "GUI.h"

/* functions */

/**********************************************************/
/* name: render_header
** description:  */
void render_header(void)
{
    printf("********************************************************\n");
    printf("**          WELCOME TO THE STREAM LISTENNER!          **\n");
    printf("**                                                    **\n");
    printf("** Input only valid from this list:                   **\n");
    printf("** streams                                            **\n");
    printf("** status                                             **\n");
    printf("** display on/off                                     **\n");
    printf("** format ascii/hex                                   **\n");
    printf("** debug on/off                                       **\n");
    printf("** tree                                               **\n");
    printf("** exit                                               **\n");
    printf("** help (extra)                                       **\n");
    printf("** clear (extra)                                      **\n");
    printf("********************************************************\n");
}

/**********************************************************/
/* name: render_insert
** description:  */
void render_insert(void)
{
    printf("iamroot >> ");
}
/**********************************************************/
/* name: render_help
** description:  */
void render_help(void)
{
    printf("********************************************************\n");
    printf("** Commands available are:                            **\n");
    printf("**    streams, status, tree, exit, help, clear,       **\n");
    printf("**    debug on/off, display on/off, format ascii/hex  **\n");
    printf("********************************************************\n");
}
