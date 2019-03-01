/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the user includes */
#include "startup.h"
#include "UI.h"
#include "tcp_api.h"

/* our main */

int main(int argc, char const *argv[]) {
    /* auxiliary variables delcaration*/
    root_connection my_connect;

    /* check stream ID */
    if( argc < 2)
    {
        /* did not connect to stream, show UI*/
    }
    else
    {
        /* connect to stream and show UI*/
        /* stream connect returns success and fills pointer to connection */
        if(connect_to_stream(&my_connect, argc, argv))
        {
            printf("Unexpected error\n");
            return -1;
        }
        /* show the UI */
    }

    return 0;
}
