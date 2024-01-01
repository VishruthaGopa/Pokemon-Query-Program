/**
   @file  server.h
*/
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_PORT 6002
#define MAX_POKEMON 801
#define MAX_LINE_LENGTH 100

// Struct to hold Pokemon description
typedef struct {
    int id;
    char *name;
    char *type1;
    char *description;
} Pokemon;

struct SearchArguments {
    Pokemon *pokemonData;      // array of structs with all the 800 pokemon
    char *searchType; // field to store the search type
    char *serializedData;
    int lengthSerializedData;

    int totalPokemonInData; // number of pokemon in pokemonData
    pthread_mutex_t mutex;
};  

void *searchByTypeSerializedData(void *args);
void readPokemonFile(struct SearchArguments *searchArgs);
void freePokemonData(struct SearchArguments *searchArgs);

#endif // SERVER_H
