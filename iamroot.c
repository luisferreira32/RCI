/* the built in includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the user includes */

/* structs definition -- wasn't this supposed to be on header?*/
struct streamid_s
{
    char name[45];
    char ip[15];
    int port;
}
streamid_default =
{
    "\0",
    "0.0.0.0",
    -1
};
typedef struct streamid_s streamid;

struct root_connection_s
{
    streamid this_stream;
    char ipaddr[15];
    int tport;
    int uport;
    char rsaddr[15];
    int rsport;
    int tcpsessions;
    int bestpops;
    int tsecs;
    bool bopt;
    bool dopt;
    bool hopt;
}
root_connection_default =
{
    streamid_default // it's popping error!
}
;
typedef struct root_connection_s root_connection;

int main(int argc, char const *argv[]) {
    /* auxiliary variables delcaration*/

    /* check stream ID */
    if( argc < 2)
    {
        /* did not connect to stream, show UI*/
    }
    else
    {
        /* connect to stream and show UI*/
    }

    /* check input arguments and options*/
    if( argc > 3)
    {
        for (size_t i = 0; i < argc; i++)
        {

            switch (argv[i])
            {
                /* options with arguments option */
                case '-i':
                    /* treat it */
                    i++;
                    break;

                /* options WITHOU arguments*/
                case '-d':
                    /* treat it*/
                    break;
            }
        }
    }
    return 0;
}
