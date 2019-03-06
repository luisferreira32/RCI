/* includes of external libs */

/* own header include */
#include "startup.h"

/* functions */
int connect_to_stream(root_connection * my_connect, int argc, const char ** argv)
{
    for (size_t i = 1; i < argc; i++)
    {
        /* options with arguments option */
        if (strcmp(argv[i], "-i") == 0)
        {
          /* treat it */
          printf("-i option with %s\n", argv[i+1]);
          i++;
        }
        // ...
        /* options WITHOUT arguments*/
        else if (strcmp(argv[i], "-d") == 0)
        {
          // do something else
        }
        /* more else if clauses */
        else /* default: */
        {
            printf("--help  option for command list\n");
        }
    }

    return false;
}
