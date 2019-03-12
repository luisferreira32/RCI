/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* the user includes */
#include "startup.h"
#include "UI.h"

/* our main */

int main(int argc, char const *argv[])
{
    /* auxiliary variables delcaration*/
    iamroot_connection my_connect;
    client_interface my_ci;

    /* first set the strcut to it's values */
    if(set_connection(&my_connect, &my_ci, argc, argv))
    {
        printf("[LOG] Application will terminate\n");
        return -1;
    }

    /* connect to stream OR send stream list and kill */

    /* show UI */
    if(show_UI(&my_connect, &my_ci))
    {
        printf("[LOG] Application will terminate\n");
        return -1;
    }

    return 0;
}
