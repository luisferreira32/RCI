/* includes of external libs */

/* own header include */
#include "startup.h"

/* functions */
bool connect_to_stream(root_connection * my_connect, int argc, const char ** argv)
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

            /* options WITHOUT arguments*/
            case '-d':
                /* treat it*/
                break;
        }
    }
}
