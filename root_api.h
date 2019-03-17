#ifndef R_API_H_
#define R_API_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "structs.h"
#include "udp_api.h"
#include "access_server.h"

int run_request(char * , char * , size_t , iamroot_connection *, bool );
int process_answer(char * , iamroot_connection *, peer_conneciton *, bool);


#endif
