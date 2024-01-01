/**
   @file  client.h
*/

#ifndef CLIENT_H
#define CLIENT_H

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6002

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

struct SaveResultsThreadArgs {
    char *serializedData;
    char ***newFiles;
    int *numNewFiles;
    pthread_mutex_t mutex;
};

//void saveResults(char *serializedData, char ***newFiles, int *numNewFiles);
void *saveResults(void *args);

#endif // CLIENT_H
