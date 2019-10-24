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
#include <fcntl.h>

#define BUFFER_SIZE 4096

void GetSite(char *url, char *buffer); //retrieve website data from URL
int CheckResp(char *buffer); //check for 200 OK response code
char *WriteToCache(char *buffer, char *url); //if 200 OK is found, write webpage to file
int CountListLines(); //count the number of lines in list.txt
int IsCached(char *url); //rewrite or update the list
//void ReadFromCache(char *url); //return the appropriate file from cache
int ReadBlacklist(char *url); //check blacklist for a given URL

