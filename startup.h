#ifndef STARTUP_H_
#define STARTUP_H_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "structs.h"

/* defines */
#define DTCPPORT 58000
#define DUDPPORT 58000
#define DROOTIP "193.136.138.142"
#define DUDPROOTPORT 59000
#define DTCPSESSIONS 1
#define DBESTPOPS 1
#define DTSECS 5
#define DFORMAT "ascii"


/* funcitons definitios */
void set_default(root_connection *);
int set_connection(root_connection *, int, const char **);
int is_ip(char *);


#endif
