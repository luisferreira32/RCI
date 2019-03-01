/* includes of external libs */

/* own header include */
#include "tcp_api.h"

/* functions definitions */

/* CLIENT SIDE */

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
int tcp_create(void)
{
    struct sockaddr_in server_addr;
    int port = 8000;
    int inet_reciv = -1;

    /*opens a stream socket to listen to our applications*/
    if((inet_reciv = socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("Inet socket creation ");
        return -1;
    }

    /*sets the address of our inet socket and binds it*/
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(inet_reciv,(const struct sockaddr *) &server_addr, sizeof(struct sockaddr_in))<0)
    {
        perror("Inet socket bind ");
        return -1;
    }

    //printf("The address is %s on port %d \n",inet_ntoa(server_addr.sin_addr),port );
    return inet_reciv;
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
    if(accept(server_fd, (struct sockaddr_in *) client_addr, &client_addr_size)<0)
    {
        perror("Failed to accept connection ");
        return -1;
    }
    /*check if address is complete*/
    if(client_addr_size < sizeof(struct sockaddr_in))
    {
        printf("Incomplete client address \n");
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
