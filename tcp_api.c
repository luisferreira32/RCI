/* own header include */
#include "tcp_api.h"

/* functions definitions */

/* CLIENT SIDE */

/************************************************************************************************/
/**** tcp_connect ****
INPUT - DNS or IP and port of server
OUTPUT - connection socket file descriptor (or -1 in case of error)
*/
int tcp_client(char * dns, char * ip, int port)
{
    int connect_socket = -1;
    struct addrinfo hints,*res;
    struct sockaddr_in server;
    char port_buffer[10];

    if(sprintf(port_buffer, "%d", port) <0)
    {
        perror("[ERROR] Getting port ");
        return -1;
    }
    port_buffer[9] = '\0';

    /* connect based on DNS*/
    if(ip == NULL && dns != NULL)
    {
        memset(&hints,0,sizeof(hints));
        hints.ai_family=AF_INET;
        hints.ai_socktype=SOCK_STREAM;
        hints.ai_flags=AI_NUMERICSERV;

        if (getaddrinfo (dns,port_buffer,&hints,&res))
        {
            perror("[ERROR] Get socket addr info ");
            return -1;
        }

        server = *((struct sockaddr_in *) res->ai_addr);
        freeaddrinfo(res);
    }
    /* connect based on IP */
    else if(ip != NULL)
    {
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        if(inet_aton(ip, &(server.sin_addr))==0)
        {
            printf("[LOG] Invalid IP address %s", ip);
            return -1;
        }
    }
    else
    {
        printf("[LOG] Need IP or DNS for UDP connection\n");
        return -1;
    }

    if((connect_socket = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("[ERROR] Inet socket creation ");
        return -1;
    }

    if(connect(connect_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))<0)
    {
        perror("[ERROR] Failed to connect to server ");
        return -1;
    }

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
		perror("[ERROR] Failed to close connection normally ");
	}
}

/* BASIC PROTOCOL */

/************************************************************************************************/
/**** tcp_send **** to send
INPUT -  server conneciton fd, message to send, lenght of memory pointed by buff
OUTPUT - size of memory recieved by server
*/
int tcp_send(int server_fd, void *buf, size_t count, bool debug)
{
    int bytes_sent = 0;

    /*send the message*/
    if((bytes_sent = send(server_fd, (const void *)buf, count, 0)) <0 )
    {
        perror("[LOG] Client failed to send message ");
        return -1;
    }

    if (debug == true)
    {
        printf("[DEBUG] TCP message of %d bytes sent\n", bytes_sent);
    }

    return 0;
}

/************************************************************************************************/
/**** tcp_recv **** to recieve msg
INPUT -  server connection fd, buffer to recieve, lenght of memory pointed by buffer
OUTPUT - size of memory recieved
*/
int tcp_recv(int server_fd, void *buf, size_t count, bool debug)
{
    int size_recv = 0;

    /*count the number of bytes copied*/
    if((size_recv = recv(server_fd, buf, count,0)) < 0)
    {
        perror("[ERROR] Client failed to recv size of message ");
        return -1;
    }

    /*check if we can support it*/
    if(size_recv > (int)count)
    {
    	printf("[LOG] Buffer overflowed \n");
    }

    /* debug option */
    if(debug == true)
    {
        printf("[DEBUG] TCP message with %d bytes recieved\n", size_recv);
    }

    /* returns 0 if orderly closed */
    return size_recv;
}

/* SERVER SIDE */


/************************************************************************************************/
/**** tcp_server **** for server to open business
INPUT -  port to open tcp access point
OUTPUT - socked file descriptor or error -1
*/
int tcp_server(int port)
{
    int connect_socket = -1;
    char port_buffer[10];
    struct addrinfo hints,*res;

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_NUMERICSERV;

    if(sprintf(port_buffer, "%d", port) <0)
    {
        perror("[ERROR] Getting udp port ");
        return -1;
    }
    port_buffer[9] = '\0';

    if (getaddrinfo (NULL,port_buffer,&hints,&res))
    {
        perror("[ERROR] Get socket addr info ");
        return -1;
    }

    if((connect_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
    {
        perror("[ERROR] Inet socket creation ");
        freeaddrinfo(res);
        return -1;
    }

    if(bind(connect_socket,res->ai_addr, res->ai_protocol)<0)
    {
        perror("[ERROR] Inet socket bind ");
        freeaddrinfo(res);
        return -1;
    }

    if(listen(connect_socket, 5))
    {
        perror("[ERROR] Failed to start listening ");
        freeaddrinfo(res);
        return -1;
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
    if((client_fd=accept(server_fd, (struct sockaddr *)client_addr, &client_addr_size))<0)
    {
        perror("[ERROR] Failed to accept connection ");
        return -1;
    }

    /*check if address is complete*/
    if(client_addr_size != sizeof(struct sockaddr_in))
    {
        printf("[LOG] Unexpected client address \n");
    }

    return client_fd;
}
