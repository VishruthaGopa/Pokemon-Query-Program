# Pokemon Query Program
***Author:*** Vishrutha Gopa

## Introduction

This C program explores the properties of Pokemon by allowing users to perform queries on a CSV file containing Pokemon data. The program consists of a client-server architecture, where the server (Pokemon Property Server - PPS) is responsible for managing Pokemon data, and the client (Pokemon Query Client - PQC) enables users to interact with the server remotely. Utilizing multi-threading, the program enhances responsiveness.

## How to Run

1. **Compile the PPS program using a Linux terminal:**
   ```bash
   make server
2. Start the PPS program:
   ```bash
    ./server
- The PPS prompts the Administrator to enter the name of the file containing Pokemon descriptions. Enter the filename or full directory path.
- The PPS starts listening on localhost (127.0.0.1) port 80.

3. Compile the PQC program:
   ```bash
    make client
4. Start the PQC program:
   ```bash
    ./client
- The PQC establishes a connection to the PPS.
- The PQC displays a menu with options:
    Type search
    Save results
    Exit the program

### Menu
* Type Search (Option 1): Search Pokemon by type (Type 1).
* Save Results (Option 2): Save accumulated query results to a file.
* Exit Program (Option 3): Terminate the PQC program.