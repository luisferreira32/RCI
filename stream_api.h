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
#include "startup.h"

/* functions definitions */
int recieve_listeners(int );
int accept_children(int );
int connect_stream(char *, int);
int stream_recv(int, char *, bool);
int stream_recv_downstream(char *,peer_conneciton*, iamroot_connection *, client_interface *, int, pop_list **);
int stream_recv_upstream(int,char *, peer_conneciton* , iamroot_connection * , bool,int, pop_list **);
int stream_welcome(iamroot_connection * , peer_conneciton * , bool );
int stream_redirect(int, char *, bool);
int stream_status(peer_conneciton *, bool);
int stream_data(char *,peer_conneciton*, client_interface*);
int stream_popquery(peer_conneciton *,iamroot_connection *, bool);
int stream_treequery(peer_conneciton *, bool);
int stream_treereply(peer_conneciton * ,iamroot_connection * , bool );


#endif
