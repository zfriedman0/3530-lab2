#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>
//#include <stdbool.h>

#define BUFFER_SIZE 4096

void GetSite(char *url, char *buffer);
//void MakeFiles(); //create cache and blacklist files
int CheckResp(char *buffer, char *url);
int CheckBlacklist(); //check blacklist for URL, if there return 1, else return 0
void WriteToCache(char *url); //write the URL to cache if not blacklisted

