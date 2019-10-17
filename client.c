/*
  Created by Zach Friedman - zmf0010 - zacheryfriedman@my.unt.edu

  Description: When compiled and run with the included server.c file, this client
  is used to send an HTTP request to any valid URL. HTTP communication is handled
  by the proxy, which will return the results to the client.
*/

#include "header.h"

int main(int argc, char **argv) {

    
    int sockfd, portno, n;
    int len = sizeof(struct sockaddr);
    char recvline[40960], buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    //usage: ./client <port number>
    //url: <some URL>
    if(argc != 2) {
        printf("usage: ./client <port number>\n");
        exit(1);
    }

    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    portno = atoi(argv[1]);
    sockfd=socket(AF_INET, SOCK_STREAM, 0);

    //error checking
    if(sockfd < 0) {
        perror("socket() error");
        exit(1);
    }

    //clear the buffer, necessary details for connecting to proxy
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(portno); // Server port number
 
    /* Convert IPv4 and IPv6 addresses from text to binary form */
    inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr));
 
    /* Connect to the server, error check */
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
        perror("connect() error");
        exit(1);
    }

    //prints after successful connection
    printf("Connected to server...\n");

    while(1) {

        //get a URL from the user
        printf("url: ");
        bzero(buffer, BUFFER_SIZE);
        scanf("%s", buffer);

        //write the URL to the buffer (send to proxy)
        n = write(sockfd, buffer, strlen(buffer));

        //error checking
        if(n < 0) {
            perror("write() error");
            exit(1);
        }

        //clear buffer for a new URL, clear recvline to ensure proper output
        bzero(buffer, BUFFER_SIZE);
        bzero(recvline, 40960);

        //read the results of the HTTP request from the proxy server
        n = read(sockfd, recvline, 40960);

        //check for error, print output
        if(n < 0) {
            perror("read() error");
            exit(1);
        } else {
            printf("\n\n");
            printf("Server says: \n%s", recvline);
            printf("\n\n");
        }
    }

    //close the connection to the proxy server
    close(sockfd);
    
    return 0;
}
