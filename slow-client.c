/*
 * Send a very slow HTTP request piece by piece
 * Usage: request <server> <port> <path>
 * Vlakkies Schreuder
 * Mar 28, 2022
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//
//  Print message to stderr and exit
//
void fatal(const char* format , ...)
{
    va_list args;
    va_start(args,format);
    vfprintf(stderr,format,args);
    va_end(args);
    exit(1);
}

//
//  Write null terminated string to socket
//  Then sleep 1s to slow things down
//
void writestr(int sock,char* str)
{
    int len = strlen(str);
    while (len>0)
    {
        int i = write(sock,str,len);
        if (i<0) fatal("Write failed on %s\n",str);
        str += i;
        len -= i;
    }
    sleep(1);
}

//
//  Main program - send request
//
int main(int argc, char *argv[])
{
    if (argc != 4) fatal("Usage: %s <host> <port> <path>\n",argv[0]);
    char* path = argv[3];
    if (!strlen(path)) fatal("path cannot be blank\n");

    //  Resolve host
    char* host = argv[1];
    struct hostent* he = gethostbyname(host);
    if (!he) fatal("Cannot resolve host name %s\n",host);
    //  Get port
    int port = atoi(argv[2]);
    if (port<1 || port>65535) fatal("Invalid port %d\n",port);

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) fatal("socket open failed\n");
    //  Set receive timeout to 2s
    struct timeval tv = {2,0};
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(struct timeval*)&tv,sizeof(struct timeval));
    // Connect to server
    struct sockaddr_in serveraddr;
    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy(he->h_addr_list[0],&serveraddr.sin_addr.s_addr,4);
    serveraddr.sin_port = htons((unsigned short)port);
    if (connect(sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))) fatal("Failed to connect to %s:%d\n",host,port);

    // Send HTTP GET
    writestr(sock,"GET ");
    writestr(sock,path);
    writestr(sock," HTTP/1.1\r\n");
    // Send Host:
    writestr(sock,"Host: ");
    writestr(sock,host);
    writestr(sock,"\r\n");
    // No keepalive
    writestr(sock,"Connection: Close\r\n");
    // Send blank line
    writestr(sock,"\r\n");

    //  Dump reply to stdout
    //  End on timeout or EOF
    char ch;
    while (read(sock,&ch,1)==1)
        fputc(ch,stdout);

    //  Close connection
    close(sock);
    return 0;
}


