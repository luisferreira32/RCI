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
#include "root_api.h"

/* defines */
/* defaults */
#define DTCPPORT 58000
#define DUDPPORT 58000
#define DROOTIP "193.136.138.142"
#define DUDPROOTPORT 59000
#define DTCPSESSIONS 1
#define DBESTPOPS 1
#define DTSECS 5
#define DFORMAT "ascii"


/* funcitons definitios */
void set_default(iamroot_connection *, client_interface *);
void set_myselfdefault(peer_conneciton *);
int set_connection(iamroot_connection *, client_interface *, int, const char **);
int is_ip(char *);
void display_help(void);


#endif
