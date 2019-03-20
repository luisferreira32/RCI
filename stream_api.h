#ifndef STREAM_API_H_
#define STREAM_API_H_

/* external includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

/* our libs */
#include "tcp_api.h"
#include "structs.h"

/* functions definitions */
int recieve_listeners(int );
int accept_children(int );
int connect_stream(char *, int);
int stream_recv(peer_conneciton*, client_interface *);

#endif
