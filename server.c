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
          char *m = WriteToCache(buffer, url);

          FILE *cachefd = fopen(m, "r");

          if(!cachefd) {
            perror("Error opening cached webpage");
            exit(1);
          }

          n = write(conn_fd, buffer, strlen(buffer));
        } else if(ReadBlacklist(url) == 1){
          n = write(conn_fd, blocked, strlen(blocked));
        } else {
          n = write(conn_fd, buffer, strlen(buffer));
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
char *WriteToCache(char *buffer, char *url) {

  //used to hold the file name of cached web page
  //YYYYMMDDhhmmss.txt
  char *timeString = ".txt"; //this should technically be timeTemp
  char timeInt[512]; //this is formatted into YYYYMMDDhhmmss
  char *timeTemp = malloc(sizeof(char) * 512); //this is improperly named, as it is the final YYYYMMDDhhmmss.txt
  char *listEntry;

  //necessary time structure
  time_t rawtime;
  struct tm * timeinfo;

  //filling out time struct object
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  //file descriptors
  FILE *cachefd;
  FILE *linkfd;

  //if 200 OK is found
  if(CheckResp(buffer) == 1) {

    //format timeInt using struct object
    strftime(timeInt, 100, "%Y%m%d%H%M%S", timeinfo);

    //copy YYYYMMDDhhmmss into final string
    //concat .txt
    //makes YYYYMMDDhhmmss.txt
    strcpy(timeTemp, timeInt);
    strcat(timeTemp, timeString);

    /* WRITE WEBPAGE TO A CACHE FILE */
    cachefd = fopen(timeTemp, "w");

    //error checking
    if(!cachefd) {
      perror("Error opening cache file");
      exit(1);
    }

    //print buffer to timestamp file
    fputs(buffer, cachefd);

    //close the file
    fclose(cachefd);

    /* WRITE URL TO LIST.TXT */
    linkfd = fopen("list.txt", "a");

    //error checking
    if(!linkfd) {
      perror("Error opening list file");
      exit(1);
    }

    //number of lines in list.txt
    int num = CountListLines();

    //as long as number of lines is <= 5, the URL gets written
    //TO ADD: else, rewrite list.txt with most recent URL at beginning
    if(IsCached(url) == 0 && num < 5){
      fprintf(linkfd, "%s %s\n", url, timeInt);
    }
    //else if IsCached(url) == 0 && num > 5 --> RewriteCache
    //else --> return ??

    //close the file
    fclose(linkfd);

    //returns YYYYMMDDhhmmss.txt for use in main to read from cache
    return timeTemp;
  }
  free(timeTemp);
}

/*
  Description: This helper function is used to count the number of lines in list.txt.
*/
int CountListLines() {

  FILE *fp = fopen("list.txt", "r");
  int count = 0;
  char c;
  char *charToCheck;

  //error checking
  if(!fp) {
    perror("Error opening list.txt");
    exit(1);
  }

  //count lines in list.txt
  for(c = getc(fp); c != EOF; c = getc(fp)) {
    if(charToCheck == "\n") {
      count++;
    }
  }

  return count;
}

int IsCached(char *url) {

  FILE *listfd = fopen("list.txt", "r");
  char urlToFind[512];

  if(listfd == NULL) {
    perror("Error opening list.txt");
    exit(1);
  }

  while(fgets(urlToFind, 512, listfd)) {
    if(strstr(urlToFind, url) != NULL) {
      return 1;
    } else {
      return 0;
    }
  }

  fclose(listfd);
}

/*
  Description: This function checks the blacklist file for a given URL.
*/
int ReadBlacklist(char *url) {

  //file descriptor
  FILE *blkfd = fopen("blacklist.txt", "r");

  //temporary URL buffer
  char urlToCheck[512]; //pretty sure there's a better way to do this (all URL buffers should be larger/dynamic)

  //check if the URL is found within the file
  while(fgets(urlToCheck, 512, blkfd)) {
    if(strstr(urlToCheck, url) != NULL) { //if the URL IS found
      return 1;
    } else { //if the URL is NOT found
      continue;
      return 0;
    }
  }
}