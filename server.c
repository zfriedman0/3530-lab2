/*
  Created by Zach Friedman - zmf0010 - zacheryfriedman@my.unt.edu

  Description: When compiled and run with the included client.c file, this simple server
  acts as a proxy between a single client application and some website of the user's choosing. 
  This is achieved by first connecting to a client, and then to the web server that hosts
  the user-specified domain.
*/

#include "header.h"

int main(int argc, char **argv) {
 
    char url[200], buffer[BUFFER_SIZE];
    int listen_fd, conn_fd, portno, clilen, n;
    struct sockaddr_in servaddr, cliaddr;

    char *blocked = "This website has been blocked.";

    //usage: ./pserver <port number>
    if(argc != 2) {
      printf("usage: ./pserver <port number>\n");
      exit(1);
    }
 
    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    //error checking
    if(listen_fd < 0) {
      perror("socket(1) error");
      exit(1);
    }
 
    //necessary socket details
    bzero(&servaddr, sizeof(servaddr));
    portno = atoi(argv[1]);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portno); /* Pick another port number to avoid conflict with other students */
 
    /* Binds the above details to the socket, also error checking */
	  if(bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
      perror("bind(1) error");
      exit(1);
    }
	  
    /* Start listening to incoming connections */
	  if(listen(listen_fd, 10) < 0) {
      perror("listen(1) error");
    }

    printf("Server created successfully. Listening...\n");

    while(1) {

      //a char array to hold the URL 
      char url[300];

      //easy-to-use variable for client address size
      clilen = sizeof(cliaddr);

      /* Accepts an incoming connection */
	    conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);

      //error checking
      if(conn_fd < 0) {
        perror("accept(1) error...\n");
        exit(1);
      }

      //prints after client connects successfully
      printf("Client connected...\n");

      //creates an infinite loop in which the server will do the dirty work
      while(1) {
        bzero(url, 200);

        //read the url the user wants to visit
        n = read(conn_fd, url, 200);

        //error checking
        if(n < 0) {
          perror("read(1) error...\n");
        }

        //report the url requested by the user
        printf("Client URL: %s\n", url);

        //call GetSite(), which fetches the website data from the URL
        GetSite(url, buffer);

        //if the site is not blacklisted, write response to a file and to client
        //else, notify client that site is blocked
        if(ReadBlacklist(url) == 0) {
          WriteToCache(buffer, url);
          n = write(conn_fd, buffer, strlen(buffer));
        } else {
          n = write(conn_fd, blocked, strlen(blocked));
        }
      }

      //close the connection when the job is done
      close(conn_fd);
    }
}

/*
  Description: This function essentially repeats the same process used in main(), 
  but with the web server that hosts the site at the given URL on port 80.
*/
void GetSite(char *url, char *buffer) {
  
  //char array to hold the GET HTTP request
  char getRequest[300];

  //format the GET HTTP request that will be sent to the web server
  sprintf(getRequest, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", url);

  //"get" file descriptor, port number, and misc. variable.
  //same necessary details used to open a connection to the given URL on port 80.
  int get_fd, get_portno, get_n;
	struct sockaddr_in get_addr;
	struct hostent *host;
	get_portno = 80; //use port 80 for HTTP communication

  //opening a new socket
  get_fd = socket(AF_INET, SOCK_STREAM, 0);

  //error checking
  if(get_fd < 0) {
    perror("socket(2) error");
    exit(1);
  }

  //does a lookup on the URL, maps to appropriate IPv4 address.
  host = gethostbyname(url);

  //checking that the host actually exists
	if (host == NULL) {
		printf("Invalid host...\n");
		exit(0);
	}

  //specifying family and port number of socket
  get_addr.sin_family = AF_INET;
  get_addr.sin_port = htons(get_portno);
  memcpy(&get_addr.sin_addr, host->h_addr_list[0], host->h_length);

  //attempt to connect to the web server at the other end of port 80
  if(connect(get_fd, (struct sockaddr*) &get_addr, sizeof(get_addr)) < 0) {
    perror("connect(2) error");
  }

  //write GET request to the web server
  get_n = write(get_fd, getRequest, strlen(getRequest));

  //error checking
  if(get_n < 0) {
    perror("write(2) error");
  }

  //clear the buffer
  bzero(buffer, BUFFER_SIZE);

  //read the response from the web server
  get_n = read(get_fd, buffer, BUFFER_SIZE);

  //error checking
  if(get_n < 0) {
    perror("read(2) error");
  }

  //close the connection
  close(get_fd);
}

/*
  Description: This function checks for a 200 OK response code within the buffer.
*/
int CheckResp(char *buffer) {

  //checking for "200 OK" response code
  if(strstr(buffer, "HTTP/1.1 200 OK") != NULL) {
    return 1;
  } else {
    return 0;
  }
}

/*
  Description: This function writes a webpage to a cache file.
*/
void WriteToCache(char *buffer, char *url) {

  //used to hold the file name of cached web page
  //YYYYMMDDhhmmss.txt
  char *timeString;
  char *listEntry;

  //necessary time structure
  time_t rawtime;
  struct tm * timeinfo;

  //filling out time struct object
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  //format timeString using struct object
  strftime(timeString, 100, "%Y%m%d%H%M%S.txt", timeinfo);

  //file descriptors
  FILE *cachefd; //= fopen(timeString, "w");
  FILE *linkfd;  //= fopen("list.txt", "a");

  //error checking
  if(cachefd == NULL) {
    perror("Error opening cache file");
    exit(1);
  }

  //if 200 OK is found
  if(CheckResp(buffer) == 1) {

    /* WRITE WEBPAGE TO A CACHE FILE */
    cachefd = fopen(timeString, "w");

    if(!cachefd) {
      perror("Error opening cache file");
      exit(1);
    }

    fputs(buffer, cachefd);
    fclose(cachefd);

    /* WRITE URL TO LIST.TXT */
    linkfd = fopen("list.txt", "a");

    //error checking
    if(!linkfd) {
      perror("Error opening list file");
      exit(1);
    }

    fprintf(linkfd, "%s %s\n", url, timeString);
    fclose(linkfd);
  }
}

/*
  Description: This function checks the blacklist file for a given URL.
*/
int ReadBlacklist(char *url) {

  //file descriptor
  FILE *blkfd = fopen("blacklist.txt", "r");

  //check if the URL is found within the file
  if(strstr("blacklist.txt", url) != NULL) { //if the URL is NOT found
    return 1;
  } else { //if the URL IS found
    return 0;
  }
}