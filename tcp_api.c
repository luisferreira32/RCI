/* own header include */
#include "tcp_api.h"

/* functions definitions */

/* CLIENT SIDE */

/************************************************************************************************/
/**** tcp_connect ****
INPUT - arguments, IP and port of server
OUTPUT - connection socket file descriptor (or -1 in case of error)
*/
int tcp_connect(char * dns, int port)
{
    int connect_socket = -1;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_NUMERICSERV;

    if (getaddrinfo (dns,port,&hints,&res))
    {
        perror("Get socket addr info ");
        return -1;
    }

    if((connect_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    if(connect(connect_socket, res->ai_addr, res->ai_addrlen)<0)
    {
        perror("Failed to connect to server ");
        return -1;
    }

    freeaddrinfo(res);
    return connect_socket;
}

/************************************************************************************************/
/**** tcp_disconnect ****
INPUT - server file descriptor
OUTPUT - none
*/
void tcp_disconnect(int server_fd)
{
	if(close(server_fd))
	{
		perror("Failed to close connection normally ");
	}
}

/* BASIC PROTOCOL */

/************************************************************************************************/
/**** tcp_send_check **** to send and check reception of message
INPUT -  server conneciton fd, message to send, lenght of memory pointed by buff
OUTPUT - size of memory recieved by server
*/
int tcp_send_check(int server_fd, void *buf, size_t count)
{
    size_t bytes_sent = 0;

    /*send information to the server so he prepares to recieve the message*/
    if(send(server_fd, (const void *)&count, sizeof(size_t), 0) == -1)
    {
        perror("Client failed to send request ");
        return 0;
    }

    /*send the message*/
    if(send(server_fd, (const void *)buf, count, 0) == -1)
    {
        perror("Client failed to send message ");
        return 0;
    }

    /*count the number of bytes recieved by the other parts*/
    if(recv(server_fd, &bytes_sent, sizeof(size_t),0) == -1)
    {
        perror("Client failed to recv size copied ");
        return 0;
    }

    return (int) bytes_sent;
}

/************************************************************************************************/
/**** tcp_recv_check **** for client to recieve info from server and checksend
INPUT -  server connection fd, buffer to recieve, lenght of memory pointed by buffer
OUTPUT - size of memory recieved
*/
int tcp_recv_check(int server_fd, void *buf, size_t count)
{
    size_t msg_size = 0;
    size_t size_recv = 0;

    /*count the number of bytes copied*/
    if(recv(server_fd, &msg_size, sizeof(size_t),0) == -1)
    {
        perror("Client failed to recv size of message ");
        return 0;
    }

    /*check if we can support it*/
    if(msg_size > count)
    {
    	printf("Buffer overflowed \n");
    	size_recv = -1;
    }
    else
    {
        /*recieve the information*/
        if((size_recv=recv(server_fd, buf, count,0)) < 0)
        {
            perror("Client failed to recv the requested ");
            size_recv = -1;
        }
    }

    /* confirm that it was recieved or send erro < 0*/
    if(send(server_fd, (const void *)&size_recv, sizeof(size_t), 0) < 0)
    {
        perror("Client failed to send request ");
        return 0;
    }

    return size_recv;
}

/* SERVER SIDE */


/************************************************************************************************/
/**** tcp_create **** for server to open business
INPUT -  none
OUTPUT - socked file descriptor or error -1
*/
int tcp_create(int port)
{
    int connect_socket = -1;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_NUMERICSERV;

    if (getaddrinfo (NULL,port,&hints,&res))
    {
        perror("Get socket addr info ");
        return -1;
    }

    if((connect_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    if(bind(connect_socket,res->ai_addr, res->ai_protocol)<0)
    {
        perror("Inet socket bind ");
        return -1;
    }

    if(listen(connect_socket, 5))
    {
        perror("Failed to start listening ");
    }

    freeaddrinfo(res);
    return connect_socket;
}

/************************************************************************************************/
/**** tcp_accept **** for server to accept clients
INPUT -  server file descriptor, pointer to memory for client address
OUTPUT - new socket file descriptor for client communicaiton
*/
int tcp_accept(int server_fd, struct sockaddr_in * client_addr)
{
    socklen_t client_addr_size;
    int client_fd = -1;

    /*accept incoming connecion and save client address*/
    client_addr_size = sizeof(struct sockaddr_in);
    if(client_fd=(accept(server_fd, (struct sockaddr_in *) client_addr, &client_addr_size))<0)
    {
        perror("Failed to accept connection ");
        return -1;
    }
    
    /*check if address is complete*/
    if(client_addr_size != sizeof(struct sockaddr_in))
    {
        printf("Unexpected client address \n");
    }

    return client_fd;
}

/************************************************************************************************/
/**** tcp_destroy **** destroys inet sockets
INPUT - socket file descriptor
OUTPUT - none
*/
void tcp_destroy(int fd)
{
    if(close(fd))
    {
        perror("INET socket close ");
    }
}
