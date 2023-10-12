#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "client.h"
#include "utils.h"
#include "network.h"

bool enterGame() {
    if (!initClient()) {
        printf("\nError while initializing the client!");
        return FALSE;
    }

    // Start listening to everything the server sends
    pthread_t pid;
    if (pthread_create(&pid, NULL, receiveData, NULL)) {
        printf("\nError while creating the thread!\n");
        return FALSE;
    }

    // Wait the signal to send the player data
    while((getDataReceived().data) == NULL);

    // Set the player info
    setPlayer();

    // Show game settings 
    char* temp;
    while((temp = getDataReceived().data) == NULL);
    printf("%s", temp);

    free(temp);

    printf("\x1b[1;33m\n\nWait the game master to start the game...\x1b[1;0m");

    // Play the game
    playTurn();

    // Close the client connection
    closeClient();

    return TRUE;
}

bool sendPlayerData(Player player) {
    // Encode the player data
    char* data = (char*) malloc(375);
    int dataLen = sprintf(data, "%s>%c", player.playerName, player.useAdvices ? 'Y' : 'N');
    data = (char*) realloc(data, dataLen);
    
    // Send the data
    sendData(data);

    free(data);

    return TRUE;
}
