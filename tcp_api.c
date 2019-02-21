#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>


/************************************************************************************************/
/**** tcp_connect ****
INPUT - arguments, IP and port of server
OUTPUT - connection socket file descriptor (or -1 in case of error)
*/
int tcp_connect(char * IP_char, char * port_char)
{
    struct sockaddr_in inet_source;
    int connect_socket = -1, port = -1;

    /*get the port*/
    if(sscanf(port_char, "%d", &port)==0)
    {
    	perror("Failed to get port of parents ");
    	return -1;
    }

    /*opens a INET stream socket*/
    if((connect_socket = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("Failed to create server connection socket ");
        return -1;
    }

    /*set address and connect to server*/
    inet_source.sin_family = AF_INET;
    inet_source.sin_port = htons((int)port);
    inet_aton(IP_char,&inet_source.sin_addr);
    if(connect(connect_socket, (const struct sockaddr *) &inet_source, sizeof(inet_source))<0)
    {
        perror("Failed to connect to server ");
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
		perror("Failed to close connection normally ");
	}
}

/************************************************************************************************/
/**** tcp_send_check **** to send and check reception of message
INPUT -  server conneciton fd, message to send, lenght of memory pointed by buff
OUTPUT - size of memory recieved by server
*/
int tcp_send_check(int server_fd, void *buf, size_t count)
{
    size_t bytes_copied = 0;

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

    /*count the number of bytes copied*/
    if(recv(server_fd, &bytes_copied, sizeof(size_t),0) == -1)
    {
        perror("Client failed to recv size copied ");
        return 0;
    }

    return (int) bytes_copied;
}

/************************************************************************************************/
/**** tcp_recv **** for client to recieve info from server
INPUT -  server connection fd, buffer to recieve, lenght of memory pointed by buffer
OUTPUT - size of memory recieved
*/
int tcp_recv(int server_fd, void *buf, size_t count)
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
    	return 0;
    }

    /*recieve the information*/
    if((size_recv=recv(server_fd, buf, count,0)) == -1)
    {
        perror("Client failed to recv the requested ");
        return 0;
    }

    return size_recv;
}