/* includes of external libs */

/* own header include */
#include "access_server.h"

/* functions */

/* access server stuff */
int open_access_server(int port, peer_conneciton * myself)
{
    if((myself->accessfd = udp_server(port)) < 0)
    {
        printf("[ERROR] Failed to create file descriptor\n");
    }
    return 0;
}

/* register on root in tsecs function */
