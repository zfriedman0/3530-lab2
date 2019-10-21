#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 4096

void GetSite(char *url, char *buffer); //retrieve website data from URL
int CheckResp(char *buffer); //check for 200 OK response code
void WriteToCache(char *buffer, char *url); //if 200 OK is found, write webpage to file
int ReadCache(char *url); //determine if the number of URLs in list.txt exceeds 5
int ReadBlacklist(char *url); //check blacklist for a given URL

