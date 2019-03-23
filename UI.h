#ifndef UI_H_
#define UI_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

/* our lives */
#include "structs.h"
#include "GUI.h"
#include "root_api.h"
#include "stream_api.h"

/* defines */

/* struct definitions */

/* funcitons definitios */
int read_command(char *, iamroot_connection *, client_interface *, peer_conneciton *);

#endif
