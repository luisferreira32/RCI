#ifndef STREAM_LIFE_
#define STREAM_LIFE_

/* includes of external libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

/* my libs*/
#include "structs.h"
#include "udp_api.h"
#include "root_api.h"

/* functions */
/* root */
int open_access_server(int, peer_conneciton *);
int refresh_root(iamroot_connection * , bool );
/* non root */
int pop_request(char*, size_t, char*, int, bool);

#endif
