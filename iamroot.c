/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* the user includes */
#include "startup.h"
#include "UI.h"
#include "tcp_api.h"

/* our main */

int main(int argc, char const *argv[])
{
    /* auxiliary variables delcaration*/
    root_connection my_connect;

    /* first set the strcut to it's values */
    if(set_connection(&my_connect, argc, argv))
    {
        printf("Application will terminate\n");
        return -1;
    }

    return 0;
}
