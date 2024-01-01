/**
   @file  server.c
*/

#include "server.h"

int main() {
    int                 serverSocket, clientSocket;
    struct sockaddr_in  serverAddress, clientAddr;
    int                 status, addrSize, bytesRcv;
    char                buffer[30];
  
    pthread_t           searchThread;
    struct              SearchArguments searchArgs;

    // Read Pokemon data from the CSV file
    readPokemonFile(&searchArgs);
    
    // Initialize searchArgs with the required data
    searchArgs.lengthSerializedData = 0;
    searchArgs.serializedData = " ";

    // Initialize the mutex
    if (pthread_mutex_init(&(searchArgs.mutex), NULL) != 0) {
        printf("Failed to initialize mutex.\n");
        exit(-1);
    }

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        printf("*** SERVER ERROR: Could not open socket.\n");
        exit(-1);
    }

    // Setup the server address
    memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

    // Bind the server socket
    status = bind(serverSocket,  (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (status < 0) {
        printf("*** SERVER ERROR: Could not bind socket.\n");
        exit(-1);
    }

    // Set up the line-up to handle up to 5 clients in line 
    status = listen(serverSocket, 5);
    if (status < 0) {
        printf("*** SERVER ERROR: Could not listen on socket.\n");
        exit(-1);
    }

    printf("\n***Remember to enter \"./client\" in another Terminal to establish connection.\n");

    // Wait for clients now
    while (1) {
        addrSize = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
        if (clientSocket < 0) {
            printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
            exit(-1);
        }
        printf("SERVER: Received client connection.\n\n");

        // Go into infinite loop to talk to client
        while (1) {
            // Get the message from the client
            bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
            buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
            printf("SERVER: Received client request: %s\n", buffer);

            // Pass the search type to the searchByType function
            searchArgs.searchType = strdup(buffer);

            if (strcmp(buffer,"3") == 0)
            break;

            // Create and start the search thread
            pthread_create(&searchThread, NULL, searchByTypeSerializedData, (void*)&searchArgs);

            // Wait for the search thread to finish
            pthread_join(searchThread, NULL);

            // Send the length of serialized data to the client
            char lengthBuffer[20];
            snprintf(lengthBuffer, sizeof(lengthBuffer), "%d", searchArgs.lengthSerializedData);
            //printf("SERVER: Sending Length: %s\n", lengthBuffer);
            send(clientSocket, lengthBuffer, strlen(lengthBuffer), 0);
            
            // Wait for client's confirmation to send the data
            bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
            buffer[bytesRcv] = '\0'; // Null-terminate the received data
            if (strcmp(buffer, "Waiting for data") != 0) {
                printf("SERVER: Unexpected response from the client.\n");
            }

            // Send the serialized data to the client
            send(clientSocket, searchArgs.serializedData, strlen(searchArgs.serializedData), 0);
            //printf("SERVER: Serialized data: %s\n", searchArgs.serializedData);
            printf("SERVER: Sent serializedData to client.\n\n");


        }

        // Close this client's socket
        printf("SERVER: Closing client connection.\n");
        close(clientSocket);

        
        if (strcmp(buffer, "3") == 0 && searchArgs.lengthSerializedData ==0) {
            return 0;
        }
        

        if (strcmp(buffer, "3") == 0) {
            break;
        }
    }

    // Cancel the thread and destroy the mutex before exiting the program
    pthread_cancel(searchThread);

    pthread_mutex_destroy(&(searchArgs.mutex));

    freePokemonData(&searchArgs);


    // Don't forget to close the sockets!
    close(serverSocket);
    printf("SERVER: Shutting down.\n");
}

// Function to read Pokemon data from the CSV file an populate the pokemanData array with the data
void readPokemonFile(struct SearchArguments *searchArgs) {
    FILE *file;
    int numPokemon = 0;
    char *filename;
    int fileFound = 0;

    while (!fileFound) {
        printf("Enter the name of the file containing the Pokemon descriptions (or 'exit' to quit): ");
        scanf("%ms", &filename);

        // Hardcoded file name for convenience. Comment this!
        // char *filename = "pokemon.csv";
        
        if (strcmp(filename, "exit") == 0) {
            printf("Exiting the program...\n");
            exit(0);
        }

        if ((file = fopen(filename, "r")) == NULL) {
            printf("Pokemon file is not found. Try again.\n");
        } else {
            fileFound = 1;
        }
    }

    // Allocate memory for pokemonData array
    searchArgs->pokemonData = malloc(sizeof(Pokemon) * MAX_POKEMON);
    if (searchArgs->pokemonData == NULL) {
        printf("Memory allocation failed for pokemonData.\n");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    fgets(line, sizeof(line), file); // Skip header line


    while (fgets(line, sizeof(line), file)) {
        Pokemon pokemon;
        pokemon.name = NULL;
        pokemon.type1 = NULL;
        pokemon.description = NULL;

        sscanf(line, "%d,%m[^,],%m[^,],%m[^\n]", &pokemon.id, &pokemon.name, &pokemon.type1, &pokemon.description);

        (searchArgs->pokemonData)[numPokemon++] = pokemon;
    }

    fclose(file);

    searchArgs->totalPokemonInData = numPokemon;  // Store the number of Pokemon read

    free(filename);
}

// Function to search for Pokemon by type
void *searchByTypeSerializedData(void *args) {
    struct SearchArguments *searchArgs = (struct SearchArguments *)args;
    Pokemon *pokemonData = searchArgs->pokemonData;
    char *result = " ";

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&(searchArgs->mutex));

    int totalLength = 0;
    for (int i = 0; i < searchArgs->totalPokemonInData; i++) {
        if (strcasecmp((pokemonData[i]).type1, searchArgs->searchType) == 0) {
            totalLength += snprintf(NULL, 0, "%d,%s,%s,%s|", pokemonData[i].id, pokemonData[i].name, pokemonData[i].type1, pokemonData[i].description);
        }
    }

    if (totalLength == 0) {
        // No matches found
        printf("No data found.\n");
    } else {
        // Matches found, allocate memory for the result string
        result = (char*)malloc(totalLength + 1);
        if (result == NULL) {
            printf("Memory allocation failed for serialized string.\n");
            exit(1);
        }

        int currentPosition = 0;
        for (int i = 0; i < searchArgs->totalPokemonInData; i++) {
            if (strcasecmp((pokemonData[i]).type1, searchArgs->searchType) == 0) {
                int written = snprintf(result + currentPosition, totalLength - currentPosition, "%d,%s,%s,%s|", pokemonData[i].id, pokemonData[i].name, pokemonData[i].type1, pokemonData[i].description);
                currentPosition += written;
            }
        }

        result[totalLength] = '\0';

        // Append trailing "|"
        strcat(result, "|");
    }

    // Unlock the mutex after accessing shared data
    pthread_mutex_unlock(&(searchArgs->mutex));

    // Store the dynamically allocated serialized data in the SearchArguments structure
    searchArgs->serializedData = result;
    searchArgs->lengthSerializedData = totalLength;
    
    pthread_exit(NULL);
}


// Free the memory allocated
void freePokemonData(struct SearchArguments *searchArgs) {
    for (int i = 0; i < searchArgs->totalPokemonInData; i++) {
        free(searchArgs->pokemonData[i].name);
        free(searchArgs->pokemonData[i].type1);
        free(searchArgs->pokemonData[i].description);
    }
    free(searchArgs->pokemonData);
    free(searchArgs->searchType);
    free(searchArgs->serializedData);
}
