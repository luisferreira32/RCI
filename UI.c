/* includes of external libs */

/* own header include */
#include "UI.h"
#include "GUI.h"

/* functions */

int show_UI(void)
{
    /* variables */
    int quit = 0;
    char command_buffer[20];

    /* our user interface calling protocols */
    render_header();

    while (!quit)
    {
        render_insert();
        scanf("%s",command_buffer );
        printf("%s\n", command_buffer );
        quit = 1;
    }

    return 0;
}
