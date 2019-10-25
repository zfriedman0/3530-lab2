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

        if(CheckResp(buffer) == 1 && IsCached(url) == 0) { //URL is not blacklisted OR cached

          //generate new timestamp and filename
          char *time = Timestamp();
          char *cached = CacheFilename(time);

          WriteToCache(buffer, url, time, cached); //write to cache, which puts the URL in list.txt and timestamps
          n = write(conn_fd, buffer, strlen(buffer)); //write the response directly to the client
          printf("Write to cache\n\n");

        } else if(CheckResp(buffer) == 1 && IsCached(url) == 1){ //URL is not blacklisted but IS cached
          ReadFromCache(conn_fd); //read the response from the cache file
          printf("Read from cache\n\n");

        } else if(ReadBlacklist(url) == 1) { //URL is blacklisted
          n = write(conn_fd, blocked, strlen(blocked));

        } else {
          n = write(conn_fd, buffer, strlen(buffer)); //write the response directly to the client
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
  Description: This function creates and returns a timestamp in the form of YYYYMMDDhhmmss
*/
char *Timestamp() {

  char *timeInt = malloc(sizeof(char) * 512); //this is formatted into YYYYMMDDhhmmss

  //necessary time structure
  time_t rawtime;
  struct tm * timeinfo;

  //filling out time struct object
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  //format timeInt using struct object
  strftime(timeInt, 100, "%Y%m%d%H%M%S", timeinfo);

  return timeInt; //returns YYYYMMDDhhmmss

  free(timeInt); //free memory
}

/*
  Description: This function creates a file name out of a given timestamp.
*/
char *CacheFilename(char *timestamp) {

  //used to hold the file name of cached web page
  //YYYYMMDDhhmmss.txt
  char *suffix = ".txt";
  char *timeString = malloc(sizeof(char) * 512); //the final YYYYMMDDhhmmss.txt

  //copy YYYYMMDDhhmmss into final string
  //concat .txt
  //makes YYYYMMDDhhmmss.txt
  strcpy(timeString, timestamp);
  strcat(timeString, suffix);

  return timeString; //returns YYYYMMDDhhmmss.txt

  free(timeString); //free memory
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
  ISSUES: 
          To make this more modular, creating the cache file and the list.txt entry should be done separately.
          Having the function use buffer might be dangerous.
*/
void WriteToCache(char *buffer, char *url, char *timeRaw, char *timePrc) {

  //timeRaw = YYYYMMDDhhmmss
  //timePrc --> "timeProcessed" = YYYYMMDDhhmmss.txt

  //file descriptors
  FILE *cachefd;
  FILE *linkfd;

  /* WRITE WEBPAGE TO A CACHE FILE */
  cachefd = fopen(timePrc, "w");

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
  if(num < 5){
    fprintf(linkfd, "%s %s\n", url, timeRaw); //Issue: Figure out how to list all timestamps of attempts to access the URL
  }
  //else if IsCached(url) == 0 && num > 5 --> RewriteCache
  //else --> return ??

  //close the file
  fclose(linkfd);
}

/*
  Description: This helper function is used to count the number of lines in list.txt.
  ISSUES: Not sure this is working properly.
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

/*
  Description: This helper function checks if a URL exists in list.txt
*/
int IsCached(char *url) {

  //open the list.txt file
  FILE *listfd = fopen("list.txt", "r");
  char urlToFind[512]; //buffer to hold incoming URLs

  //error checking
  if(listfd == NULL) {
    perror("Error opening list.txt");
    exit(1);
  }

  //searches over strings within list.txt
  while(fgets(urlToFind, 512, listfd)) { //search line-by-line
    if(strstr(urlToFind, url) != NULL) { //match found
      return 1;
    } else { //no match found
      return 0;
    }
  }

  //close list.txt
  fclose(listfd);
}

/*
  Description: This function separates a URL from its timestamp in list.txt
*/
char *SplitString(char *stringToSplit) {

  char *stringToModify = malloc(sizeof(char) * 512);
  char *delim = " "; //the delimeter to look for

  strcpy(stringToModify, stringToSplit); //copy passes string into modifiable string buffer
  stringToModify = strtok(stringToModify, delim); //starting index = URL

  while(stringToSplit != NULL) { //subsequent calls to strtok get next token
    stringToModify = strtok(NULL, delim); 

    //scrub newlines
    char *newline = strchr(stringToModify, '\n');

    if(newline) {
      *newline = 0;
    }

    return stringToModify; //return the timestamp
  }

  free(stringToModify); //free memory
}

/*
  Description: This function searches list.txt and grabs the timestamp to print the contents of the cached webpage
*/
void ReadFromCache(int sockfd) {

  //char *cachedFile = CacheFilename(timestamp);
  char urlBuffer[512]; //placeholder for URL
  char *timeBuffer = {0}; //placeholder for timestamp being searched for
  char *tempFilename = {0}; //placeholder for name of cache file
  char *contents; //buffer to hold contents of file
  long length;

  //open list.txt
  FILE *listfd = fopen("list.txt", "r");

  if(!listfd) { //error checking
    perror("Error opening list.txt");
    exit(1);
  }
    
  while(fgets(urlBuffer, 512, listfd)) { //searching over lines in list.txt
    timeBuffer = SplitString(urlBuffer); //split urlBuffer to get timestamp @ each line
    tempFilename = CacheFilename(timeBuffer); //create a filename out of the timestamp

    if(strstr(urlBuffer, timeBuffer) != NULL) { //timestamp is found in list.txt
        FILE *cachefd = fopen(tempFilename, "r"); //open the cache file

        //error checking
        if(!cachefd) {
          perror("Error opening specified cache file");
          exit(1);
        }

        /*
          The following code borrows from http://www.fundza.com/c4serious/fileIO_reading_all/
        */

        fseek(cachefd, 0L, SEEK_END); //get number of bytes in file
        length = ftell(cachefd);
        fseek(cachefd, 0L, SEEK_SET); //reset file position indicator to beginning of file

        contents = (char *)calloc(length, sizeof(char)); //grab sufficient memory for buffer to hold contents of file

        if(contents == NULL) {
          perror("Memory error");
          exit(1);
        }

        fread(contents, sizeof(char), length, cachefd); //copy text into contents buffer
        int n = write(sockfd, contents, strlen(contents)); //write to socket

        fclose(cachefd); //close cache file

        free(contents); //free memory

        /*
          The above code borrows from http://www.fundza.com/c4serious/fileIO_reading_all/
        */
    }

  fclose(listfd); //close list.txt
  }
}

/*
  Description: This function checks the blacklist file for a given URL.
*/
int ReadBlacklist(char *url) {

  //file descriptor
  FILE *blkfd = fopen("blacklist.txt", "r");

  //temporary URL buffer
  char urlToCheck[512]; //pretty sure there's a better way to do this (all URL buffers should be larger/dynamic?)

  //check if the URL is found within the file
  while(fgets(urlToCheck, 512, blkfd)) {
    if(strstr(urlToCheck, url) != NULL) { //match
      return 1;
    } else { //no match
      continue;
      return 0;
    }
  }

  fclose(blkfd);
}