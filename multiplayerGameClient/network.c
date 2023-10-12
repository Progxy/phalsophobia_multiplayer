#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "client.h"
#include "utils.h"
#include "network.h"

bool enterGame() {
    // Init the connection to the server
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
    while((getDataReceived()) == NULL);

    // Set the player info
    setPlayer();

    // Show game settings 
    char* temp;
    while((temp = getDataReceived()) == NULL);
    printf("%s", temp);

    free(temp);

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
