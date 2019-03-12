#ifndef UI_H_
#define UI_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "structs.h"
#include "GUI.h"

/* defines */

/* struct definitions */

/* funcitons definitios */
int show_UI(iamroot_connection *, client_interface * );
int read_command(char *, client_interface *);

#endif
