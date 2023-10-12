#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "server.h"
#include "utils.h"

int startGame() {
    // Load the server
    if(!loadServer()) {
        printf("\nError loading the server!");
        return FALSE;
    }

    // Wait for the users to enter the server, and show the connected ones
    int totalPlayers = createServerList();

    // Start the threads to listen to all the data sent from all the clients
    pthread_t pids[totalPlayers];
    int clientsIds[totalPlayers];
    for (int i = 0; i < totalPlayers; i++) {
        clientsIds[i] = i;
        if (pthread_create(pids + i, NULL, receiveData, (void*)(clientsIds + i))) {
            printf("Error: failed creating the thread!\n");
        }
    }

    // Set the game
    set(totalPlayers + 1);

    // Set the game master player
    setPlayers(0, "");

    // Request the player info to all the users
    for (int i = 0; i < totalPlayers; i++) {
        // Send the signal to send the player to the user
        if (!sendData(i + 1, "SPI")) {
            printf("\nError sending the game settings!");
            return FALSE;
        }
        
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");

        printf("\nWaiting to receive the player data...\n");

        // Await the player info
        char* playerData;
        while((playerData = getDataReceived().data) == NULL);

        // Set the player using the data received from the user
        setPlayers(i + 1, playerData);

        free(playerData);

    }
    
    // Send the current game settings
    char* gameSettings = (char*) malloc(2500);
    char* tempInfo = showGameSettings();

    // Regex to clear the terminal.
    int currentLen = sprintf(gameSettings, "\e[1;1H\e[2J%s\x1b[1;33m\n\nWait the game master to start the game...\x1b[1;0m", tempInfo);
    gameSettings = (char*) realloc(gameSettings, currentLen + 1);

    for (int i = 0; i < totalPlayers; i++) {
        if (!sendData(i + 1, gameSettings)) {
            printf("\nError, while sending the game settings!");
            return FALSE;
        }
    }

    printf("%s", gameSettings);

    free(tempInfo);
    free(gameSettings);
    
    {
        char confirm;
        printf("\x1b[1;33m\n\nPress ENTER to continue: \x1b[1;0m");
        scanf("%c", &confirm);
    }

    // Reset the data before the game
    resetData();

    // Play the game
    playGame();

    // Close the server connection
    closeServer();

    return TRUE;
}
