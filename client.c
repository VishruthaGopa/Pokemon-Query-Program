/**
   @file  client.c
*/

#include "client.h"

int main() {
    int                 clientSocket;
    struct sockaddr_in  clientAddress;
    int                 status, bytesRcv;

    int                 choice;    // stores user input from keyboard
    char                buffer[30];   // stores sent and received data //(from online textbook)
    int                 allocatedSize = 0;
    int                 intialArraySize = 30;
    char                **newFiles = NULL; // Dynamic array to store names of new files
    int                 numNewFiles = 0;    // Number of new files created
    int                 numSuccessQueries = 0;
    char                *serializedData = NULL;
    pthread_t           saveThread;
    struct              SaveResultsThreadArgs saveArgs;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0) {
        printf("*** CLIENT ERROR: Could open socket.\n");
        exit(-1);
    }

    // Setup address
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    clientAddress.sin_port = htons((unsigned short) SERVER_PORT);

    // Connect to server
    status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
    if (status < 0) {
        printf("*** CLIENT ERROR: Could not connect.\n");
        exit(-1);
    }

    // Dynamic array for serialized data
    serializedData = malloc(intialArraySize * sizeof(char));
    if (serializedData == NULL) {
        printf("error: failed to allocate memory for serializedData.\n");
        exit(0);
    }

    // Initialize saveArgs with the required data
    saveArgs.serializedData = serializedData;
    saveArgs.newFiles = &newFiles;
    saveArgs.numNewFiles = &numNewFiles;

    // Initialize the mutex
    if (pthread_mutex_init(&(saveArgs.mutex), NULL) != 0) {
        printf("Failed to initialize mutex.\n");
        exit(-1);
    }


    // Go into loop to commuincate with server now
    while (1) {

        printf("\nMenu:\n");
        printf("1. Type search\n");
        printf("2. Save results\n");
        printf("3. Exit the program\n");
        printf("Enter your choice: ");

        // Handle invalid user input
        if (scanf("%d", &choice) != 1) {
            printf("Invalid choice. Please try again. Select 1, 2, or 3.\n");
            while (getchar() != '\n');
            continue;
        }

        // Check if the choice is valid
        if (choice < 1 || choice > 3) {
            printf("Invalid choice. Please try again. Select 1, 2, or 3.\n");
            continue;
        }

        switch (choice) {
            case 1:
                // Get the search type from the user
                printf("Enter the type to search: ");
                scanf("%s", buffer);

                // Send command string to server
                printf("CLIENT: Sending \"%s\" to server.\n", buffer);
                send(clientSocket, buffer, strlen(buffer), 0);

                // Receive the length of serialized data from the server
                char lengthBuffer[20];
                bytesRcv = recv(clientSocket, lengthBuffer, sizeof(lengthBuffer), 0);
                lengthBuffer[bytesRcv] = '\0';
                int lengthStr = atoi(lengthBuffer);
                //printf("CLIENT: Recieved length \"%d\" from server.\n", lengthStr);

                // Send a confirmation message to the server to indicate that the client is ready to receive the data
                char* confirmation = "Waiting for data";
                send(clientSocket, confirmation, strlen(confirmation), 0);

                // Dynamically allocate memory for the currentData based on the length received from the server
                char* currentData = (char*)malloc(lengthStr + 1); // +1 for null-terminator
                
                // Receive the serialized data from the server
                bytesRcv = recv(clientSocket, currentData, lengthStr+1, 0);
                currentData[bytesRcv] = 0; // put a 0 at the end so we can display the string
                printf("CLIENT: Recieved type search data from server.\n");
                //printf("CLIENT: currentData \"%s\" from server.\n", currentData);


                if(lengthStr!=0){
                    // Dynamically size the serializedData array to append the received string with the existing char array
                    int newTotalSize = allocatedSize + lengthStr;
                    serializedData = (char*)realloc(serializedData, newTotalSize + 1);

                    // Append the received serialized data to the existing serializedData
                    strcat(serializedData, currentData);
                    allocatedSize = newTotalSize;
                }

                //printf("\nAppended data: %s \n\n", serializedData);
                numSuccessQueries++;
                free(currentData);
                break;
            case 2:
                saveArgs.serializedData = serializedData;

                pthread_create(&saveThread, NULL, saveResults, (void *)&saveArgs);

                // Wait for the thread to finish its execution
                pthread_join(saveThread, NULL);

                break;
            case 3:
                // Display number of queries completed successfully
                printf("\nTotal number of queries completed successfully: %d\n\n", numSuccessQueries);

                // Display the names of new files created during the session
                printf("Here are the names of the files created during the session: \n");
                for (int i = 0; i < numNewFiles; i++) {
                    printf("%s\n", newFiles[i]);
                    free(newFiles[i]);
                }
                free(newFiles);
                free(serializedData);

                // Exit the program
                printf("\nExiting the program.\n");

                // Send command string to server
                snprintf(buffer, sizeof(buffer), "%d", choice); // Convert integer to string
                printf("CLIENT: Sending \"%s\" to server.\n", buffer);
                send(clientSocket, buffer, strlen(buffer), 0);

                // Destroy the mutex before exiting the program
                pthread_mutex_destroy(&(saveArgs.mutex));

                close(clientSocket); // Close socket
                printf("CLIENT: Shutting down.\n");
                return 0;                
        }        
    } 

    printf("CLIENT: Shutting down.\n");
    return 0;
}



// Function to save query results to a file
void *saveResults(void *args) {
    struct SaveResultsThreadArgs *saveArgs = (struct SaveResultsThreadArgs *)args;
    char *serializedData = saveArgs->serializedData;
    char ***newFiles = saveArgs->newFiles;
    int *numNewFiles = saveArgs->numNewFiles;

    char *filename;
    printf("Enter the name of the file to save the query results: ");
    scanf("%ms", &filename);

    FILE *file = fopen(filename, "w");

    while (file == NULL) {
        printf("Unable to create the new file. Please enter the name of the file again: ");
        scanf("%ms", &filename);
        file = fopen(filename, "w");
    }

    // Create a copy of the serializedData to tokenize
    char *dataCopy = strdup(serializedData);


    // Write each Pokemon description on a separate line
    char *token = strtok(dataCopy, "|");
    while (token != NULL) {
        fprintf(file, "%s\n", token);
        token = strtok(NULL, "|");
    }

    fclose(file);

    // Lock the mutex before accessing shared data (newFiles array and numNewFiles)
    pthread_mutex_lock(&(saveArgs->mutex));

    // Add the filename to the newFiles array
    (*numNewFiles)++;
    *newFiles = realloc(*newFiles, (*numNewFiles) * sizeof(char *));
    (*newFiles)[(*numNewFiles) - 1] = strdup(filename);

    // Unlock the mutex after updating the shared data (newFiles array and numNewFiles)
    pthread_mutex_unlock(&(saveArgs->mutex));

    free(filename);
    free(dataCopy);

    pthread_exit(NULL);
}