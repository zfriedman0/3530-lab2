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
char *Timestamp(); //create a timestamp as YYYYMMDDhhmmss
char *CacheFilename(char *timestamp); //make a filename out of a timestamp
int CheckResp(char *buffer); //check for 200 OK response code
void WriteToCache(char *buffer, char *url, char *timeRaw, char *timePrc); //if 200 OK is found, write webpage to file
int CountListLines(); //count the number of lines in list.txt
int IsCached(char *url); //rewrite or update the list
void ReadFromCache(char *url, char *timestamp); //return the appropriate file from cache (this function definition assumes no two timestamps will ever be the same)
int ReadBlacklist(char *url); //check blacklist for a given URL

