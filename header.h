#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

void GetSite(char *url, char *buffer); //retrieve website data from URL
int CheckResp(char *buffer); //check for 200 OK response code
void WriteToCache(char *url); //write the URL to cache if not blacklisted

