#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network.h"
#include "utils.h"
#include "client.h"

static const char* colorsCodes[] = {"\x1b[1;30m", "\x1b[1;31m", "\x1b[1;32m", "\x1b[1;33m", "\x1b[1;34m", "\x1b[1;35m", "\x1b[1;36m", "\x1b[1;37m", "\x1b[1;0m"};

static void printColored(char* str, ColorType color) {
    printf("%s%s%s", colorsCodes[color], str, colorsCodes[DEFAULT_COLOR]);
    return;
}

void setPlayer() {
    // Regex to clear the terminal.
    printf("\e[1;1H\e[2J");

    printColored("\n------------- PLAYER INFO -------------\n", MAGENTA);

    // Allocate the space in the heap for the player struct
    Player player = {"", INACTIVE};

    // Allocate the space for the player's name
    player.playerName = (char*) malloc(225);

    // Get the player's name
    do {
        printf("\nInsert the name to use in game%s (MAX 225 characters)%s: ", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);

        // Check if the input string contains the newline character
        char *temp;
        if ((temp = strchr(fgets((player.playerName), 225, stdin), '\n')) != NULL) {
            // Change the newline with the string terminator
            *temp = '\0';

            // Resize the size of the name array
            player.playerName = (char*) realloc(player.playerName, strlen(player.playerName) + 1);
        } else {
            // If the input string doesn't contain the newline than clean the stdin
            unsigned char c;
            while((c = getc(stdin)) != EOF) {
                if(c == '\n') {
                    break;          
                }
            }

            // Change the newline with the string terminator
            *temp = '\0';
        }
        
        // Check if the given input string is empty or not
        int isEmpty = 1;
        for (int i = 0; player.playerName[i] != '\0'; i++) {
            if ((player.playerName[i] != ' ')) {
                isEmpty = 0;
                break;
            }
        }

        if (isEmpty) {
            printColored("\nError: please insert a valid input!\n", RED);
            continue;
        } else {
            break;
        }

    } while (TRUE);

    // Ask if the player wants advice during the game
    do {
        char confirm;
        printColored("\nDo you want to receive advices during the game? (Y/N): ", YELLOW);
        scanf("%c", &confirm);

        if (confirm == 'Y') {
            player.useAdvices = ACTIVE;
            break;
        } else if (confirm == 'N') {
            player.useAdvices = INACTIVE;
            break;
        }

        printColored("\nError: please insert a valid input!\n", RED);

    } while (TRUE);

    {
        // Clean the stdin
        unsigned char c;
        while ((c = getc(stdin)) != EOF) {
            if (c == '\n'){
                break;
            }
        }

        char confirm;
        printColored("\n\nPress ENTER to continue: ", YELLOW);
        scanf("%c", &confirm);
    }

    // Send the player data to the server
    sendPlayerData(player);

    free(player.playerName);

    return;
}

static void sendInput() {
    char* temp = (char *) calloc(50, 1);
    fgets(temp, 50, stdin);
    temp = (char*) realloc(temp, strlen(temp) + 1);
    
    if (!sendData(temp)) {
        printf("\nError while sending the data to the server!");
    }

    free(temp);

    return;
}

void playTurn() {
    while (TRUE) {
        // Wait to know if it's your turn
        char* temp;
        while ((temp = getDataReceived()) == NULL);

        // If it's not your turn wait the end of the turn
        if (!strcmp(temp, "NYT")) {
            bool endGameCondition;
            
            while (!strcmp(temp, "TT") || (endGameCondition = !strcmp(temp, "TG"))) {
                while ((temp = getDataReceived()) == NULL);
                printf("%s", temp);
            }

            free(temp);

            // Check if the game ended
            if (endGameCondition) {
                return;
            }
        
        }

        do {
            // Get the instructions from the game
            while ((temp = getDataReceived()) == NULL);
            
            // Check if the user input is needed
            if (!strcmp(temp, "UI")) {
                free(temp);
                sendInput();
                continue;
            }

            // Wait to know if the turn is ended
            if (!strcmp(temp, "TT")) {
                free(temp);
                break;
            }

            // Print the info
            printf("%s", temp);
            free(temp);

        } while(TRUE);
    }
}
