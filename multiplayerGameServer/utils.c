#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "server.h"

/* INTERNALS VARIABLES INITIALIZATION AND INTERNALS FUNCTIONS DECLARATION */

static GameStates gameState = UNSET;
static int gameLevel;
static int playerCount;
static Player** players = NULL;
static MapZone* firstZone = NULL;
static MapZone* lastZone = NULL;
static int* turns = NULL;
static EvidenceType caravanEvidence[3];
static ZoneType ghostPosition;
static int ghostAppearance;
static time_t currentTime;
static int roundCount;

static const char* zoneTypeNames[] = {"CARAVAN", "KITCHEN", "LIVING_ROOM", "ROOM", "BATHROOM", "GARAGE", "BASEMENT", "-"};
static const char* objectsNames[] = {"-", "EMF", "SPIRIT_BOX", "CAMERA", "SEDATIVE", "SALT", "ADRENALINE", "HUNDRED_DOLLAR", "KNIFE", "TRANQUILLIZER", "NO_OBJECT", "EMF_EVIDENCE", "SPIRIT_BOX_EVIDENCE", "CAMERA_EVIDENCE", "NO_EVIDENCE", "EMPTY_SLOT"};
static const char* difficultiesLevels[] = {"AMATEUR", "INTERMEDIATE", "NIGHTMARE"};
static const char* colorsCodes[] = {"\x1b[1;30m", "\x1b[1;31m", "\x1b[1;32m", "\x1b[1;33m", "\x1b[1;34m", "\x1b[1;35m", "\x1b[1;36m", "\x1b[1;37m", "\x1b[1;0m"};
static const int gameLevels[] = {15, 30, 50};

/// @brief Insert a zone to the end of the list.
static void insertZone();

/// @brief Delete the last zone.
static void deleteZone();

/// @brief Print all the zones currently on the map.
static void printZones();

/// @brief Move the evidence from the player's backpack to the caravan, and set the player's position to the first zone.
/// @param playerIndex 
/// @return Return 1 if the player has an evidence, otherwise 0.
static void goToCaravan(int playerIndex);

/// @brief Print the info of the given player.
/// @param playerIndex 
/// @param currentTurn 
static void printPlayer(int playerIndex, int currentTurn);

/// @brief Print the info of the given zone.
/// @param playerIndex
/// @param currentTurn 
static void printZone(int playerIndex, int currentTurn);

/// @brief Move the given player to the next zone.
/// @param playerIndex 
static void goToNextZone(int playerIndex);

/// @brief Pick an evidence from a zone if the player has the object to pick it.
/// @param playerIndex 
static void pickEvidence(int playerIndex);

/// @brief Try to pick the object from the current player's position.
/// @param playerIndex 
static void pickObject(int playerIndex);

/// @brief Use an object from the availables in the backpack. 
/// @param playerIndex 
/// @param currentTurn 
static void useObject(int playerIndex, int currentTurn);

/// @brief Give an object to another player.
/// @param playerIndex 
/// @param currentTurn 
static void giveObjects(int playerIndex, int currentTurn);

/// @brief Remove an object from the player's backpack.
/// @param playerIndex 
/// @param currentTurn 
static void removeObject(int playerIndex, int currentTurn);

/// @brief Let the user reorginize the backpack as he wants to.
/// @param playerIndex 
/// @param currentTurn 
static void reorginizeBackpack(int playerIndex, int currentTurn);

/// @brief Generate a random number in a given range.
/// @param range 
/// @return Return a random number in the given range.
static int randomNumber(int range);

/// @brief Generate the turns for a round.
static void generateTurns();

/// @brief Check if the players win or lose.
static void checkGameStatus();

/// @brief Print the info of the objects availables. 
/// @param objects 
static char* printObjectsInfo(unsigned char objects[]);

/// @brief Print all the evidences that are in the caravan.
/// @param playerIndex 
/// @param currentTurn 
static void printEvidenceCollected(int playerIndex, int currentTurn);

/// @brief Print the info about the ghost.
/// @param playerIndex 
/// @param currentTurn 
static void printGhostInfo(int playerIndex, int currentTurn);

/// @brief Print using the given color.
/// @param str 
/// @param color 
static void printColored(char* str, ColorType color);

/// @brief Print the advice for the player.
/// @param playerIndex
static char* printAdvices(int playerIndex);

/// @brief Play the turn as the game master.
/// @param turnIndex 
static void playTurn(int turnIndex);

/* END OF INITIALIZATIONS AND DECLARATIONS */

void set(int playerNum) {
    // Reset the current time
    currentTime = 0;

    // Set the global variable player count
    playerCount = playerNum;

    // Regex to clear the terminal.
    printf("\e[1;1H\e[2J");

    // Allocate the space for the players
    players = (Player**) calloc(playerCount, sizeof(Player*));

    // Request the difficulty level
    do {
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");
        
        printColored("\n------------- DIFFICULTY LEVELS -------------\n", MAGENTA);
        printf("\n1) Amateur;");
        printf("\n2) Intermediate;");
        printf("\n3) Nightmare.");
        printf("\nChoose the difficulty level from the option above: ");
        scanf("%d", &gameLevel);

        // Check if the game level selected is valid
        if ((1 <= gameLevel) && (gameLevel <= 3)) {
            gameLevel--;
            break;
        }

        printColored("\nError: please insert a valid input!", RED);

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

    } while (TRUE);
    
    // Generate the game map
    do {
        int choice = 0;

        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");

        printColored("\n------------- MAP EDITOR -------------\n", MAGENTA);
        printf("\n1) Insert a new zone;");
        printf("\n2) Delete the last zone;");
        printf("\n3) Print the zones currently on the map;");
        printf("\n4) Close the map.");
        printf("\nChoose between the option listed above: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: 
                insertZone();
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
                break;   

            case 2: 
                deleteZone();
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
                break;            
            
            case 3: 
                printZones();
                {
                    // Clean the stdin
                    unsigned char c;
                    while((c = getc(stdin)) != EOF) {
                        if(c == '\n') {
                            break;          
                        }
                    } 

                    char confirm;
                    printColored("\n\nPress ENTER to continue: ", YELLOW);
                    scanf("%c", &confirm);
                }
                break;

            case 4:
                // Check if the map has been set
                if (firstZone == NULL) {
                    printColored("Before closing the map, set at least one zone!\n", RED);

                    {
                        // Clean the stdin
                        unsigned char c;
                        while((c = getc(stdin)) != EOF) {
                            if(c == '\n') {
                                break;          
                            }
                        } 

                        char confirm;
                        printColored("\nPress ENTER to continue: ", YELLOW);
                        scanf("%c", &confirm);
                    }
                    
                    break;
                }

                return;

            default:
                printColored("\nError: please insert a valid input!", RED);

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

                break;
        }

    } while (TRUE);

    return;
}

void setPlayers(int playerIndex, char* info) {
    // Allocate the space in the heap for the player struct
    Player* player = (Player*) malloc(sizeof(Player));

    // Initialize the player mental health to 100
    player -> mentalHealth = 100;

    // If the player is the game master ask the name and the useadvice
    if (playerIndex == 0) {
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");
        
        printColored("\n------------- PLAYER INFO -------------\n", MAGENTA);
        
        // Clean the stdin
        {
            unsigned char c;
            while((c = getc(stdin)) != EOF) {
                if(c == '\n') {
                    break;          
                }
            }   
        }

        // Allocate the space for the player's name
        player -> playerName = (char*) malloc(225);
    
        // Get the player's name
        do {
            printf("\nInsert the name of the player %s (MAX 225 characters)%s: ", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);

            // Check if the input string contains the newline character
            char *temp;
            if ((temp = strchr(fgets((player -> playerName), 225, stdin), '\n')) != NULL) {
                // Change the newline with the string terminator
                *temp = '\0';

                // Resize the size of the name array
                player -> playerName = (char*) realloc(player -> playerName, strlen(player -> playerName) + 1);
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
            for (int i = 0; player -> playerName[i] != '\0'; i++) {
                if ((player -> playerName[i] != ' ')) {
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
                player -> useAdvices = ACTIVE;
                break;
            } else if (confirm == 'N') {
                player -> useAdvices = INACTIVE;
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


    } else {
        // Allocate the space for the player's name and load the name
        int index;
        player -> playerName = (char*) malloc(225);
        for (index = 0; info[index] != '>'; index++) {
            player -> playerName[index] = info[index];
        }

        player -> playerName[index] = '\0';
        player -> playerName = (char*) realloc(player -> playerName, index);

        // Set if the player uses advices
        player -> useAdvices = (info[index + 1] == 'Y');
    }

    // Generate a random object for the player
    player -> backpack[0] = randomNumber(5) + 1;

    // Set the backpack's slots as empty
    for (int i = 1; i < 4; i++) {
        player -> backpack[i] = EMPTY_SLOT;
    }

    // Set the state of the salt protection to inactive
    player -> saltProtection = INACTIVE;

    // Set the player's position to the first zone
    player -> position = firstZone;

    // Add the player to the players array
    players[playerIndex] = player;

    return;
}

char* showGameSettings() {
    char* result = (char*) malloc(2500);
    int currentLen = 0;

    // Show the current settings
    currentLen += sprintf(result + currentLen, "%s\n------------- GAME SETTINGS -------------\n%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);

    currentLen += sprintf(result + currentLen, "\nNumber of players: %d - (", playerCount);

    for (int i = 0; i < playerCount; i++) {
        currentLen += sprintf(result + currentLen, "%s%s", players[i] -> playerName, i != (playerCount - 1) ? ", " : ")");
    }
    
    currentLen += sprintf(result + currentLen, "\nGame difficulty: %s\n", difficultiesLevels[gameLevel]);

    currentLen += sprintf(result + currentLen, "%s\n------------- CURRENT MAP -------------\n%s", colorsCodes[CYAN], colorsCodes[DEFAULT_COLOR]);

    currentLen += sprintf(result + currentLen, "\nFirst Zone: ");

    for (MapZone* scan = firstZone; scan != lastZone; scan = (scan -> nextZone)) {
        currentLen += sprintf(result + currentLen, "%s --> ", zoneTypeNames[scan -> zone]);
    }
    
    // Check if last zone is already defined
    if (lastZone != NULL) {
        currentLen += sprintf(result + currentLen, "%s", zoneTypeNames[lastZone -> zone]);
    }
    
    result = (char*) realloc(result, currentLen + 1);

    return result;
}

void resetData() {
    // Set to zero all the variables
    roundCount = 0;
    ghostPosition = NO_ZONE;
    ghostAppearance = gameLevels[gameLevel];
    for (int i = 0; i < 3; i++) {
        caravanEvidence[i] = NO_EVIDENCE;
    }

    // Deallocate the turns if already used
    if (turns != NULL) {
        free(turns);
        turns = NULL;
    }

    return;
}

static void playTurn(int turnIndex) {
    Player* player = players[0];

    // If the player has been eliminated skip his turn
    if (player == NULL) {
        return;
    }

    do {
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");

        printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (turnIndex + 1), players[0] -> playerName, colorsCodes[DEFAULT_COLOR]);

        int choice = 0;
        int turnStatus = PLAYING;

        if (player -> useAdvices) {
            printf("\n----------------------------------------------------------------------------------------------------\n");
            char* advice = printAdvices(0);
            printf("%s%s%s", colorsCodes[CYAN], advice, colorsCodes[DEFAULT_COLOR]);
            printf("\n----------------------------------------------------------------------------------------------------\n");
            free(advice);
        }

        printf("\n1) Go to the caravan to deposit all the evidence from the backpack;");
        printf("\n2) Go to the next zone;");
        printf("\n3) Pick the evidence from the current zone;");
        printf("\n4) Pick the object from the current zone;");
        printf("\n5) Use an object from the backpack;");
        printf("\n6) Skip the turn;");
        printf("\n7) Give an object to another player in the same room;");
        printf("\n8) Remove an object;");
        printf("\n9) Reorganize the backpack;");
        printf("\n10) Print the player info;");
        printf("\n11) Print the current zone info;");
        printf("\n12) Print all the evidence in the caravan;");
        printf("\n13) Print the ghost info;");
        printf("\n14) Print the game info;");
        printf("\n15) Exit the game.");
        printf("\nChoose an action from the option above: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                // If there's a ghost the player can't go to the caravan
                if (ghostPosition == (player -> position -> zone)) {
                    printColored("\nYou can't go to the caravan, because there's a ghost at your position!", YELLOW);
                } else {
                    goToCaravan(0);
                    turnStatus = FINISHED;
                }
                break;

            case 2:
                goToNextZone(0);
                turnStatus = FINISHED;
                break;

            case 3: 
                pickEvidence(0);
                break;

            case 4:
                pickObject(0);
                break;

            case 5:
                useObject(0, turnIndex);
                break;

            case 6:
                turnStatus = FINISHED; 
                printColored("\nYou have skipped your turn!", YELLOW);
                break;

            case 7:
                giveObjects(0, turnIndex);
                break;

            case 8:
                removeObject(0, turnIndex);
                break;

            case 9:
                reorginizeBackpack(0, turnIndex);
                break;

            case 10:
                printPlayer(0, turnIndex);
                break;

            case 11:
                printZone(0, turnIndex);
                break;

            case 12:
                printEvidenceCollected(0, turnIndex);
                break;

            case 13:
                printGhostInfo(0, turnIndex);
                break;

            case 14:
                // Regex to clear the terminal.
                printf("\e[1;1H\e[2J");

                printf("\n%sROUND: %d - TURN: %d\n%s", colorsCodes[MAGENTA], (roundCount + 1), (turnIndex + 1), colorsCodes[DEFAULT_COLOR]);

                // Show the current settings
                char* gameSettings = showGameSettings();

                printf("%s", gameSettings);

                free(gameSettings);

                break;

            case 15: 
                close();
                return;

            default:
                printColored("\nError: please insert a valid input!", RED);
                break;
        }

        // If the player has finished the turn go to the next player turn
        if (turnStatus == FINISHED) {
            // The probability that the mental health decrease is 20 %
            int randomNum = randomNumber(100);
            
            if (randomNum < 20) {
                player -> mentalHealth -= 15;
                printf("%s\n\nYour mental health has decreased to %d%s", colorsCodes[YELLOW], player -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
            }

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

            break;
        }

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

    } while(TRUE);
    
    return;
}

static char* requestInput(int playerTurn) {
    char* userInput;
    if (!sendData(playerTurn, "UI")) {
        printf("\nError while sending the advice!");
    }

    while((userInput = getDataReceived().data) == NULL);

    return userInput;
}

void playGame() {
    while (TRUE) {
        // Generate the turns for this round
        generateTurns();

        // Play every player's turn 
        for (int index = 0; index < playerCount; index++) {
            int playerTurn = turns[index];

            // Send to all the player the current info
            for (int i = 1; i < playerCount; i++) {
                if (i == (playerTurn - 1)) {
                    if (!sendData(i, "IS_YOUR_TURN")) {
                        printf("\nError while sending the turn info!");
                    }
                    continue;
                }

                if (!sendData(i, "NYT")) {
                    printf("\nError while sending the turn info!");
                }
            }

            // Check the status of the game
            checkGameStatus();

            // If the players win or lose end the game
            if ((gameState == WIN) || (gameState == GAME_OVER)) {

                // Regex to clear the terminal.
                printf("\e[1;1H\e[2J");

                printf("\n%s%s the players have %s!%s", gameState == WIN ? colorsCodes[GREEN] : colorsCodes[RED], gameState == WIN ? "The game ends," : "Game Over, ", gameState == WIN ? "won, congratulations" : "lost", colorsCodes[DEFAULT_COLOR]);

                // Send the info of the end of the game to the players
                char* info = (char*) malloc(125);
                int size = sprintf(info, "\e[1;1H\e[2J\n%s%s the players have %s!%s", gameState == WIN ? colorsCodes[GREEN] : colorsCodes[RED], gameState == WIN ? "The game ends," : "Game Over, ", gameState == WIN ? "won, congratulations" : "lost", colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                for (int i = 1; i < playerCount; i++) {
                    if (!sendData(i, info)) {
                        printf("\nError while sending the info!");
                    }
                }
                free(info);

                // Send every user the signal that the game has ended
                for (int i = 1; i < playerCount; i++) {
                    if (!sendData(i, "TG")) {
                        printf("\nError while sending the info!");
                    }
                }

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
                
                // Deallocate all the memory from the heap for the next game
                close();

                return;
            }

            // If the player has been eliminated skip his turn
            if (players[playerTurn] == NULL) {
                // Send the terminate turn signal
                for (int i = 1; i < playerCount; i++) {                
                    if (!sendData(i, "TT")) {
                        printf("\nError while sending info");
                    }
                }
                continue;
            } 

            do {
                // Send to all the player the current info
                char* turnInfo = (char*) malloc(125);
                int size = sprintf(turnInfo, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (index + 1), players[playerTurn] -> playerName, colorsCodes[DEFAULT_COLOR]);
                turnInfo = (char*) realloc(turnInfo, size + 1);

                for (int i = 1; i < playerCount; i++) {
                    if (!sendData(i, turnInfo)) {
                        printf("\nError while sending the turn info!");
                    }
                }

                // Deallocate the temp variable
                free(turnInfo);

                // Send to all the player the current info
                char* currentPlayerInfo = (char*) malloc(125);
                int currentLen = sprintf(currentPlayerInfo, "%s\nWait the end of the current turn!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                currentPlayerInfo = (char*) realloc(currentPlayerInfo, currentLen + 1);

                for (int i = 1; i < playerCount; i++) {
                    if (i == playerTurn) {
                        continue;
                    }
                    if (!sendData(i, currentPlayerInfo)) {
                        printf("\nError while sending the turn info!");
                    }
                }

                // Deallocate the temp variable
                free(currentPlayerInfo);

                // If the current player is the game master use his own function
                if (playerTurn == 0) {
                    playTurn(index);
                    break;
                }

                printf("\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (index + 1), players[playerTurn] -> playerName, colorsCodes[DEFAULT_COLOR]);
                printColored("\nWait the end of the current turn!", YELLOW);

                int choice = 0;
                int turnStatus = PLAYING;

                if (players[playerTurn] -> useAdvices) {
                    char* temp = printAdvices(playerTurn);
                    char spacer[] = "\n----------------------------------------------------------------------------------------------------\n";
                    char* advice = (char*) malloc(750);
                    int infoSize = sprintf(advice, "%s%s%s%s%s", spacer, colorsCodes[CYAN], temp, colorsCodes[DEFAULT_COLOR], spacer);
                    advice = (char*) realloc(advice, infoSize + 1);
                    if (!sendData(playerTurn, advice)) {
                        printf("\nError while sending the advice!");
                    }
                    free(temp);
                    free(advice);
                } else {
                    if (!sendData(playerTurn, "NO_ADVICE_SELECTED")) {
                        printf("\nError while sending the advice!");
                    }
                }

                // Send the menu info
                if (!sendData(playerTurn, "\n1) Go to the caravan to deposit all the evidence from the backpack;\n2) Go to the next zone;\n3) Pick the evidence from the current zone;\n4) Pick the object from the current zone;\n5) Use an object from the backpack;\n6) Skip the turn;\n7) Give an object to another player in the same room;\n8) Remove an object;\n9) Reorganize the backpack;\n10) Print the player info;\n11) Print the current zone info;\n12) Print all the evidence in the caravan;\n13) Print the ghost info;\n14) Print the game info;\n15) Exit the game.\nChoose an action from the option above: ")) {
                    printf("\nError while sending the advice!");
                }

                // Wait to get the input from the user
                char* userInput = requestInput(playerTurn);

                choice = atoi(userInput);

                free(userInput);

                switch(choice) {
                    case 1:
                        // If there's a ghost the player can't go to the caravan
                        if (ghostPosition == (players[playerTurn] -> position -> zone)) {
                            char* info = (char*) malloc(105);
                            int size = sprintf(info, "%s\nYou can't go to the caravan, because there's a ghost at your position!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                            info = (char*) realloc(info, size + 1);
                            if (!sendData(playerTurn, info)) {
                                printf("\nError while sending info!");
                                break;
                            }
                            free(info);
                        } else {
                            goToCaravan(playerTurn);
                            turnStatus = FINISHED;
                        }
                        break;

                    case 2:
                        goToNextZone(playerTurn);
                        turnStatus = FINISHED;
                        break;

                    case 3: 
                        pickEvidence(playerTurn);
                        break;

                    case 4:
                        pickObject(playerTurn);
                        break;

                    case 5:
                        useObject(playerTurn, index);
                        break;

                    case 6:
                        turnStatus = FINISHED; 
                        {
                            char* info = (char*) malloc(325);
                            int size = sprintf(info, "%s\nYou have skipped your turn!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                            info = (char*) realloc(info, size + 1);
                            
                            if (!sendData(playerTurn, info)) {
                                printf("\nError while sending the info!");
                            }
                            
                            free(info);
                        }
                        break;

                    case 7:
                        giveObjects(playerTurn, index);
                        break;

                    case 8:
                        removeObject(playerTurn, index);
                        break;

                    case 9:
                        reorginizeBackpack(playerTurn, index);
                        break;

                    case 10:
                        printPlayer(playerTurn, index);
                        break;

                    case 11:
                        printZone(playerTurn, index);
                        break;

                    case 12:
                        printEvidenceCollected(playerTurn, index);
                        break;

                    case 13:
                        printGhostInfo(playerTurn, index);
                        break;

                    case 14:
                        {                     
                            // Show the current settings
                            char* info = (char*) malloc(1000);
                            char* gameSettings = showGameSettings();
                            int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s%s", colorsCodes[MAGENTA], (roundCount + 1), (index + 1), players[playerTurn] -> playerName, colorsCodes[DEFAULT_COLOR], gameSettings);
                            info = (char*) realloc(info, size + 1);
                            
                            if (!sendData(playerTurn, info)) {
                                printf("\nError while sending the info!");
                            }

                            free(gameSettings);
                            free(info);
                        }

                        break;

                    case 15: 
                        close();
                        return;

                    default:
                        {                            
                            char* info = (char*) malloc(125);
                            int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                            info = (char*) realloc(info, size + 1);
                            
                            if (!sendData(playerTurn, info)) {
                                printf("\nError while sending the info!");
                            }

                            free(info);
                        }
                        
                        break;
                }

                // If the player has finished the turn go to the next player turn
                if (turnStatus == FINISHED) {
                    // The probability that the mental health decrease is 20 %
                    int randomNum = randomNumber(100);
                    
                    if (randomNum < 20) {
                        players[playerTurn] -> mentalHealth -= 15;
                        
                        if (playerTurn) {
                            char* info = (char*) malloc(125);
                            int size = sprintf(info, "%s\n\nYour mental health has decreased to %d%s", colorsCodes[YELLOW], players[playerTurn] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                            info = (char*) realloc(info, size + 1);
                            
                            if (!sendData(playerTurn, info)) {
                                printf("\nError while sending the info!");
                            }

                            free(info);
                        } else {
                            printf("%s\n\nYour mental health has decreased to %d%s", colorsCodes[YELLOW], players[playerTurn] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                        }
                    }

                    // Ask to confirm
                    char* tempInfo = (char*) malloc(150);
                    int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                    tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                    if (!sendData(playerTurn, tempInfo)) {
                        printf("\nError while sending the advice!");
                    }

                    free(tempInfo);

                    // Before going to the next turn wait that the player confirms that has read that
                    requestInput(playerTurn);

                    // Send the terminate turn signal
                    for (int i = 1; i < playerCount; i++) {                
                        if (!sendData(i, "TT")) {
                            printf("\nError while sending info");
                        }
                    }

                    break;
                }

                // Ask to confirm
                char* tempInfo = (char*) malloc(150);
                int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                if (!sendData(playerTurn, tempInfo)) {
                    printf("\nError while sending the advice!");
                }

                free(tempInfo);

                // Before going to the next turn wait that the player confirms that has read that
                requestInput(playerTurn);
            
            } while(TRUE);

        }

        roundCount++;
    }

    return;
}

void close() {
    // Deallocate all the players alive
    free(players);
    players = NULL;

    // Recursively deallocate all the element from the list until the first zone is deallocated
    while (firstZone != NULL) {
        deleteZone();
    }
    
    // Deallocate the turns if already used
    if (turns != NULL) {
        free(turns);
        turns = NULL;
    }

    // Set the game status to unset
    gameState = UNSET;

    return;
}

/* INTERNALS FUNCTIONS DEFINITION */

static void insertZone() {
    // Allocate the space for a new zone
    MapZone* newZone = (MapZone*) malloc(sizeof(MapZone));

    // Initialize the linked list if isn't already initialized
    if (firstZone == NULL) {
        // Set the first zone as the new zone created
        firstZone = newZone;
        
        // Set the last zone equal to the first zone to make the list circular
        lastZone = firstZone;
        
        // Set the last zone as the next zone
        firstZone -> nextZone = lastZone;

        // Generate the type of zone (excluding the CARAVAN type)
        firstZone -> zone = randomNumber(6) + 1;

        // Generate the object inside the zone
        int randomObject = randomNumber(6) + 6;
    
        // If the generated object is equal to 11, assign it as NO_OBJECT (= 10)
        firstZone -> zoneObject = randomObject == 11 ? randomObject - 1 : randomObject;
        
        // Set the evidence in the zone as empty
        firstZone -> evidence = 0;

        return printZones();
    }
    
    // The old last zone point to the new last zone
    lastZone -> nextZone = newZone;

    // Set the last zone as the new zone created
    lastZone = newZone;

    // Set the first zone as the next zone
    lastZone -> nextZone = firstZone;

    // Generate the type of zone (excluding the CARAVAN type)
    lastZone -> zone = randomNumber(6) + 1;

    // Generate the object inside the zone
    int randomObject = randomNumber(6) + 6;
    
    // If the generated object is equal to 11, assign it as NO_OBJECT (= 10)
    lastZone -> zoneObject = randomObject == 11 ? randomObject - 1 : randomObject;

    // Set the evidence in the zone as empty
    lastZone -> evidence = 0;

    return printZones();
}

static void deleteZone() {
    // If the first zone is NULL, than the list is empty
    if (firstZone == NULL) {
        printColored("\nThe map is already empty!", YELLOW);
        return;
    }
    
    // If the list has only one element reset the list
    if (firstZone == lastZone) {
        free(firstZone);
        firstZone = NULL;
        lastZone = NULL;
        return printZones();
    }

    // Search for the zone that points to the last zone
    MapZone* scan;
    for (scan = firstZone; (scan -> nextZone) != lastZone; scan = (scan -> nextZone));
    
    // Deallocate the last zone
    free(lastZone);

    // Set the element before the last zone as the new last zone
    lastZone = scan;

    // Set the first zone as the next zone
    lastZone -> nextZone = firstZone;

    return printZones();
}

static void printZones() {
    // If the first zone is NULL, than the list is empty
    if (firstZone == NULL) {
        printColored("\nThe map is empty!", YELLOW);
        return;
    }

    printColored("\n------------- CURRENT MAP -------------\n", CYAN);
    printf("\nFirst Zone: ");

    for (MapZone* scan = firstZone; scan != lastZone; scan = (scan -> nextZone)) {
        printf("%s --> ", zoneTypeNames[scan -> zone]);
    }
    
    // Check if last zone is already defined
    if (lastZone != NULL) {
        printf("%s", zoneTypeNames[lastZone -> zone]);
    }

    return;
}

static void goToCaravan(int playerIndex) {
    // Move all the evidence in the backpack to the caravan
    int hasEvidences = 0;

    for (int i = 0; i < 4; i++) {
        unsigned char backpackSlot = players[playerIndex] -> backpack[i];
        
        // Check if the current slot is empty or if is not of type EvidenceType
        if ((backpackSlot == EMPTY_SLOT) || (backpackSlot < 11)) {
            continue;
        } else {
            hasEvidences = 1;
            // Add the evidence to a slot determined by the type of evidence, to prevent duplicate
            caravanEvidence[backpackSlot - 11] = backpackSlot;

            // Set the backpack slot to empty
            players[playerIndex] -> backpack[i] = EMPTY_SLOT;

            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("%s\nEvidence (%s) has been left in the caravan!%s", colorsCodes[MAGENTA], objectsNames[backpackSlot], colorsCodes[DEFAULT_COLOR]);
            } else {
                char* info = (char*) malloc(450);
                int size = sprintf(info, "%s\nEvidence (%s) has been left in the caravan!%s", colorsCodes[MAGENTA], objectsNames[backpackSlot], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
        }
    }

    // Check if the player has evidences
    if (!hasEvidences) {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printColored("\nYou don't have evidences!", YELLOW);      
        } else {
            char* info = (char*) malloc(65);
            int size = sprintf(info, "%s\nYou don't have evidences!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }
    }

    // Set the player position to the first zone
    players[playerIndex] -> position = firstZone;

    // Send the info if is not the game master
    if (playerIndex == 0) {
        printColored("\nYou have been repositioned in the first zone!", MAGENTA);     
    } else {
        char* info = (char*) malloc(85);
        int size = sprintf(info, "%s\nYou have been repositioned in the first zone!%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    return;
}

static void printPlayer(int playerIndex, int currentTurn) {
    Player* player = players[playerIndex];

    // Send the info if is not the game master
    if (playerIndex == 0) {
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");
        printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
        printColored("\n------------- PLAYER INFO -------------\n", MAGENTA);
        printf("\nName: %s", player -> playerName);
        printf("\nMental Health: %d", player -> mentalHealth);
        printf("\nPosition: %s", zoneTypeNames[player -> position -> zone]);
        printf((players[playerIndex] -> useAdvices) ? "\nThe advices are active" : "\nThe advices are inactive");
        printf((players[playerIndex] -> saltProtection) ? "\nThe salt protection is active" : "\nThe salt protection is inactive");

    } else {
        char* useAdv = (players[playerIndex] -> useAdvices) ? "\nThe advices are active" : "\nThe advices are inactive";
        char* useSalt = (players[playerIndex] -> saltProtection) ? "\nThe salt protection is active" : "\nThe salt protection is inactive";
        char* info = (char*) malloc(750);
        int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- PLAYER INFO -------------\n%s\nGhost position: %s\nGhost appeareance probability: %d%%\nName: %s\nMental Health: %d\nPosition: %s%s%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR], zoneTypeNames[ghostPosition], ghostAppearance, player -> playerName, player -> mentalHealth, zoneTypeNames[player -> position -> zone], useAdv, useSalt);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }
    
    // Print the backpack's slots
    for (int i = 0; i < 4; i++) {
        int slot = player -> backpack[i];
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printf("\nSlot %d: %s", (i + 1), objectsNames[slot]);
        } else {
            char* info = (char*) malloc(250);
            int size = sprintf(info, "\nSlot %d: %s", (i + 1), objectsNames[slot]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }
    }

    return;
}

static void printZone(int playerIndex, int currentTurn) {
    MapZone* currentZone = players[playerIndex] -> position;

    // Send the info if is not the game master
    if (playerIndex == 0) {
    // Regex to clear the terminal.
    printf("\e[1;1H\e[2J");
    printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
    printColored("\n------------- ZONE INFO -------------\n", MAGENTA);
    printf("\nCurrent zone: %s", zoneTypeNames[currentZone -> zone]);
    printf("\nEvidence in the current zone: %s", objectsNames[currentZone -> evidence]);
    printf("\nObject in the current zone: %s", objectsNames[currentZone -> zoneObject]);
    printf("\nNext zone: %s", zoneTypeNames[currentZone -> nextZone -> zone]);

    } else {
        char* info = (char*) malloc(750);
        int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- ZONE INFO -------------\n%s\nCurrent zone: %s\nEvidence in the current zone: %s\nObject in the current zone: %s\nNext zone: %s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR], zoneTypeNames[currentZone -> zone], objectsNames[currentZone -> evidence], objectsNames[currentZone -> zoneObject], zoneTypeNames[currentZone -> nextZone -> zone]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    return;
}

static void goToNextZone(int playerIndex) {
    // Generate the object for the current zone if there aren't
    if ((players[playerIndex] -> position -> zoneObject) == NO_OBJECT) {
        int randomObject = randomNumber(10) + 1;
        players[playerIndex] -> position -> zoneObject = randomObject;
        
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printf("%s\nThe object in the %s has been added!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
        } else {
            char* info = (char*) malloc(125);
            int size = sprintf(info, "%s\nThe object in the %s has been added!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }
    }

    // Move the player position to the next zone
    players[playerIndex] -> position = players[playerIndex] -> position -> nextZone;

    // Send the info if is not the game master
    if (playerIndex == 0) {
        printf("%s\nYou have been repositioned in the %s!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
    } else {
        char* info = (char*) malloc(125);
        int size = sprintf(info, "%s\nYou have been repositioned in the %s!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    // Change the evidence in the zone reached by the player
    int randomEvidence = randomNumber(5) + 11;
    
    // If the generated num is equal to 15, assign it as NO_EVIDENCE (= 14)
    EvidenceType newEvidence = randomEvidence == 15 ? randomEvidence - 1 : randomEvidence;
    players[playerIndex] -> position -> evidence = newEvidence;

    // Send the info if is not the game master
    if (playerIndex == 0) {
        printf("%s\nThe evidence in the %s has been changed!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
    } else {
        char* info = (char*) malloc(125);
        int size = sprintf(info, "%s\nThe evidence in the %s has been changed!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    // Generate the object for the zone reached if there aren't
    if ((players[playerIndex] -> position -> zoneObject) == NO_OBJECT) {
        int randomObject = randomNumber(10) + 1;
        players[playerIndex] -> position -> zoneObject = randomObject;
        
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printf("%s\nThe object in the %s has been added!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
        } else {
            char* info = (char*) malloc(125);
            int size = sprintf(info, "%s\nThe object in the %s has been added!%s", colorsCodes[MAGENTA], zoneTypeNames[players[playerIndex] -> position -> zone], colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }
    }

    return;
}

static void pickEvidence(int playerIndex) {
    unsigned char currentZoneEvidence = players[playerIndex] -> position -> evidence;

    if (currentZoneEvidence == NO_EVIDENCE) {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printColored("\nThere's no object in this zone that can be picked!", YELLOW);
        } else {
            char* info = (char*) malloc(125);
            int size = sprintf(info, "%s\nThere's no object in this zone that can be picked!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }

        return;
    }

    // Check if there's the object to pick the evidence, decrement the evidence by 10 to get the object needed to pick it
    // And if there's change the object with the evidence
    for (int i = 0; i < 4; i++) {
        if ((players[playerIndex] -> backpack[i]) == (currentZoneEvidence - 10)) {
            players[playerIndex] -> backpack[i] = currentZoneEvidence;
            
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("%s\nEvidence %s has been picked!%s", colorsCodes[MAGENTA], objectsNames[players[playerIndex] -> backpack[i]], colorsCodes[DEFAULT_COLOR]);
            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nEvidence %s has been picked!%s", colorsCodes[MAGENTA], objectsNames[players[playerIndex] -> backpack[i]], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }

            // Generate a random number to check the possibility that the ghost appears
            int randomNum = randomNumber(100);
            if (randomNum < ghostAppearance) {
                // Spawn the ghost in the same zone as the current player
                ghostPosition = players[playerIndex] -> position -> zone;
                
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("%s\nThe ghost spawn in the %s zone!%s", colorsCodes[MAGENTA], zoneTypeNames[ghostPosition], colorsCodes[DEFAULT_COLOR]);
                } else {
                    char* info = (char*) malloc(125);
                    int size = sprintf(info, "%s\nThe ghost spawn in the %s zone!%s", colorsCodes[MAGENTA], zoneTypeNames[ghostPosition], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }

                // Check if there's a player in the same position as the ghost, and if so decrement his mental health
                for (int index = 0; index < playerCount; index++) {
                    // Check if the player has been eliminated
                    if (players[index] == NULL) {
                        continue;
                    }

                    // If the player has used the SALT before, then his mental health won't decrement
                    if (((players[index] -> position -> zone) == ghostPosition) && (!(players[playerIndex] -> saltProtection))) {
                        switch (gameLevel) {
                            case AMATEUR:
                                players[index] -> mentalHealth -= DECREMENT_AMATEUR;
                                break;

                            case INTERMEDIATE:
                                players[index] -> mentalHealth -= DECREMENT_INTERMEDIATE;
                                break;

                            case NIGHTMARE:
                                players[index] -> mentalHealth -= DECREMENT_NIGHTMARE;
                                break;
                        }
                        

                        // Send the info if is not the game master
                        if (playerIndex == 0) {
                            printf("%s\nThe ghost is in the same room as %s, so %s's mental health decrease to %d!%s", colorsCodes[MAGENTA], players[index] -> playerName, players[index] -> playerName, players[index] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                        } else {
                            char* info = (char*) malloc(325);
                            int size = sprintf(info, "%s\nThe ghost is in the same room as %s, so %s's mental health decrease to %d!%s", colorsCodes[MAGENTA], players[index] -> playerName, players[index] -> playerName, players[index] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                            info = (char*) realloc(info, size + 1);
                            if (!sendData(playerIndex, info)) {
                                printf("\nError while sending the info!");
                            }
                            free(info);
                        }
                    
                    }
                }
            }

            // Increment the possibility that a ghost appears (based on the difficulty)
            switch (gameLevel) {
                case AMATEUR:
                    ghostAppearance += INCREMENT_AMATEUR;
                    break;  

                case INTERMEDIATE:
                    ghostAppearance += INCREMENT_INTERMEDIATE;
                    break;        
                    
                case NIGHTMARE:
                    ghostAppearance += INCREMENT_NIGHTMARE;
                    break;
            }

            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("%s\nThe probabilities that the ghost appears have been increased to %d%% %s", colorsCodes[MAGENTA], ghostAppearance, colorsCodes[DEFAULT_COLOR]);
            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nThe probabilities that the ghost appears have been increased to %d%% %s", colorsCodes[MAGENTA], ghostAppearance, colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }

            return;
        }
    }

    // Send the info if is not the game master
    if (playerIndex == 0) {
        printColored("\nYou don't have the object to pick the evidence in this zone!", YELLOW);
    } else {
        char* info = (char*) malloc(125);
        int size = sprintf(info, "%s\nYou don't have the object to pick the evidence in this zone!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    return;
}

static void pickObject(int playerIndex) {
    // Check if there's an object in the zone
    if ((players[playerIndex] -> position -> zoneObject) == NO_OBJECT) {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printColored("\nThe current zone has no objects!", YELLOW);
        } else {
            char* info = (char*) malloc(125);
            int size = sprintf(info, "%s\nThe current zone has no objects!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }

        return;
    }

    // Check if there's an empty slot for the object
    for (int i = 0; i < 4; i++) {
        if ((players[playerIndex] -> backpack[i] == EMPTY_SLOT)) {
            // Set the object in the backpack
            players[playerIndex] -> backpack[i] = players[playerIndex] -> position -> zoneObject;

            // Set the object in this zone to none, as it has been picked
            players[playerIndex] -> position -> zoneObject = NO_OBJECT;
            
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("%s\nYou have picked the %s!%s", colorsCodes[MAGENTA], objectsNames[players[playerIndex] -> backpack[i]], colorsCodes[DEFAULT_COLOR]);
            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nYou have picked the %s!%s", colorsCodes[MAGENTA], objectsNames[players[playerIndex] -> backpack[i]], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }

            return;
        }
    }

    // Send the info if is not the game master
    if (playerIndex == 0) {
        printColored("\nThe backpack's slots are full, you can't pick the object!", YELLOW);
    } else {
        char* info = (char*) malloc(125);
        int size = sprintf(info, "%s\nThe backpack's slots are full, you can't pick the object!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    return;
}

static void useObject(int playerIndex, int currentTurn) {
    do {
        int choice = 0;
        unsigned char usableObjects[] = {0, 0, 0, 0};
        int usableObjectsCount = 0;

        // Send the info if is not the game master
        if (playerIndex == 0) {
            // Regex to clear the terminal.
            printf("\e[1;1H\e[2J");

            printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            printColored("\n------------- USABLE OBJECTS -------------\n", MAGENTA);

        } else {
            char* info = (char*) malloc(350);
            int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- USABLE OBJECTS -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }


        for (int i = 0; i < 4; i++) {
            // Print as options only the objects that aren't used for the 
            unsigned char backpackObject = players[playerIndex] -> backpack[i];
            if ((3 < backpackObject) && (backpackObject < 10)) {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("\n%d) Use the %s;", i + 1, objectsNames[backpackObject]);
                } else {
                    char* info = (char*) malloc(85);
                    int size = sprintf(info, "\n%d) Use the %s;", i + 1, objectsNames[backpackObject]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }

                // Add the object to the object effects that can be shown if requested by the user
                usableObjects[usableObjectsCount] = backpackObject;
                usableObjectsCount++;
            }
        }

        // Check if there's object to be used in the backpack
        if (usableObjectsCount == 0) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printColored("\nThere aren't object that can be used!", YELLOW);
            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nThere aren't object that can be used!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }

            return;
        }

        // Print the commands only if is the game master
        if (!playerIndex) {
            // Add an option to list all the effects of the objects availables
            printf("\n5) Print all the effects of the objects availables;");

            // Add an option to exit the menu
            printf("\n6) Exit the menu.");

            printf("\nChoose from the option above: ");
            scanf("%d", &choice);
        } else {
            // Ask to choose an option
            if (!sendData(playerIndex, "\n5) Print all the effects of the objects availables;\n6) Exit the menu.\nChoose from the option above: ")) {
                printf("\nError while sending the advice!");
            }

            // Wait the user input
            char* userInput = requestInput(playerIndex);

            choice = atoi(userInput);

            free(userInput);
        }

        // Check if the given input is between 1 and 6
        if ((1 > choice) || (choice > 6)) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printColored("\nError: please insert a valid input!", RED);
            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
            
            if (!playerIndex) {
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
            } else {
                // Ask to confirm
                char* tempInfo = (char*) malloc(150);
                int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                if (!sendData(playerIndex, tempInfo)) {
                    printf("\nError while sending the advice!");
                }
                
                free(tempInfo);
                
                // Wait the user to continue
                requestInput(playerIndex);
            }

            continue;
        } 
        
        // Check if the player chose one of the extra optons
        if (choice == 5) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                // Regex to clear the terminal.
                printf("\e[1;1H\e[2J");
                printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
                printColored("\n------------- OBJECTS INFO -------------\n", MAGENTA);
                printf("%s", printObjectsInfo(usableObjects));

            } else {
                char* info = (char*) malloc(350);
                int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- OBJECTS INFO -------------\n%s%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR], printObjectsInfo(usableObjects));
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }

                free(info);
            }
            
            // Reset the usable objects list
            for (int i = 0; i < 4; i++) {
                usableObjects[i] = 0;
            }

            if (!playerIndex) {
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
            } else {
                // Ask to confirm
                char* tempInfo = (char*) malloc(150);
                int tempInfoSize = sprintf(tempInfo, "\n\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                if (!sendData(playerIndex, tempInfo)) {
                    printf("\nError while sending the advice!");
                }
                
                free(tempInfo);
                
                // Wait the user to continue
                requestInput(playerIndex);
            }

            continue;

        } else if (choice == 6) {
            return;
        }

        switch (players[playerIndex] -> backpack[choice - 1]) {
            case 4:
                // Use the SEDATIVE object to increase the mental health by 40
                players[playerIndex] -> mentalHealth += 40;
                players[playerIndex] -> backpack[choice - 1] = EMPTY_SLOT;  
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("%s\nYou used the SEDATIVE, and your mental health has increased to %d!%s", colorsCodes[MAGENTA], players[playerIndex] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                } else {
                    char* info = (char*) malloc(250);
                    int size = sprintf(info, "%s\nYou used the SEDATIVE, and your mental health has increased to %d!%s", colorsCodes[MAGENTA], players[playerIndex] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
                return;       
            
            case 5:
                // Use the SALT to prevent a decrement of the mental health, caused by the ghost
                players[playerIndex] -> saltProtection = 1;
                players[playerIndex] -> backpack[choice - 1] = EMPTY_SLOT;
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printColored("\nYou used the SALT, the next appearence of the ghost won't affect your mental health!", MAGENTA);
                } else {
                    char* info = (char*) malloc(250);
                    int size = sprintf(info, "%s\nYou used the SALT, the next appearence of the ghost won't affect your mental health!%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
                return;            
            
            case 6:
                // Use the ADRENALINE to go to the next zone and obtain an extra turn
                players[playerIndex] -> backpack[choice - 1] = EMPTY_SLOT;
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printColored("\nYou used the ADRENALINE, and went to the next zone, obtaining an extra turn!", MAGENTA);
                } else {
                    char* info = (char*) malloc(250);
                    int size = sprintf(info, "%s\nYou used the ADRENALINE, and went to the next zone, obtaining an extra turn!%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
                return goToNextZone(playerIndex);            
            
            case 7:
                // Use the HUNDRED_DOLLAR to buy a TRANQUILLIZER or SALT
                do {
                    int option = 0;
                    // Send the info if is not the game master
                    if (playerIndex == 0) {
                        printf("\nChoose what you want to buy between: ");
                        printf("\n1) TRANQUILLIZER;");
                        printf("\n2) SALT.");
                        printf("\nInsert an option: ");
                        scanf("%d", &option);
                    } else {
                        if (!sendData(playerIndex, "\nChoose what you want to buy between: \n1) TRANQUILLIZER;\n2) SALT.\nInsert an option: ")) {
                            printf("\nError while sending the info!");
                        }

                        // Wait for the user input
                        char* userInput = requestInput(playerIndex);

                        option = atoi(userInput);

                        free(userInput);
                    }

                    switch (option) {
                        case 1:
                            players[playerIndex] -> backpack[choice - 1] = TRANQUILLIZER;
                            // Send the info if is not the game master
                            if (playerIndex == 0) {
                                printColored("\nYou bought the TRANQUILLIZER!", MAGENTA);
                            } else {
                                char* info = (char*) malloc(125);
                                int size = sprintf(info, "%s\nYou bought the TRANQUILLIZER!%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);
                                info = (char*) realloc(info, size + 1);
                                if (!sendData(playerIndex, info)) {
                                    printf("\nError while sending the info!");
                                }
                                free(info);
                            }
                            return;                    

                        case 2:
                            players[playerIndex] -> backpack[choice - 1] = SALT;
                            // Send the info if is not the game master
                            if (playerIndex == 0) {
                                printColored("\nYou bought the SALT!", MAGENTA);
                            } else {
                                char* info = (char*) malloc(125);
                                int size = sprintf(info, "%s\nYou bought the SALT!%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);
                                info = (char*) realloc(info, size + 1);
                                if (!sendData(playerIndex, info)) {
                                    printf("\nError while sending the info!");
                                }
                                free(info);
                            }
                            return;

                        default:
                            // Send the info if is not the game master
                            if (playerIndex == 0) {
                                printColored("\nError: please insert a valid input!", RED);
                            } else {
                                char* info = (char*) malloc(125);
                                int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                                info = (char*) realloc(info, size + 1);
                                if (!sendData(playerIndex, info)) {
                                    printf("\nError while sending the info!");
                                }
                                free(info);
                            }
                            break;
                    }

                } while (TRUE);

                return;            
            
            case 8:
                // Use the KNIFE and if the mental health is under 30 kill all the players in the same zone as the current player
                ZoneType currentZone = players[playerIndex] -> position -> zone;
                players[playerIndex] -> backpack[choice - 1] = EMPTY_SLOT;
                
                if ((players[playerIndex] -> mentalHealth) < 30) {
                    for (int i = 0; i < playerCount; i++) {
                        // Check if the player has been eliminated
                        if (players[i] == NULL) {
                            continue;
                        }

                        if ((i != playerIndex) && ((players[i] -> position -> zone) == currentZone)) {
                            free(players[playerIndex]);
                            players[playerIndex] = NULL;
                            
                            // Send the info if is not the game master
                            if (playerIndex == 0) {
                                printf("%s\nYou used the KNIFE, and killed %s!%s", colorsCodes[MAGENTA], players[i] -> playerName, colorsCodes[DEFAULT_COLOR]);
                            } else {
                                char* info = (char*) malloc(250);
                                int size = sprintf(info, "%s\nYou used the KNIFE, and killed %s!%s", colorsCodes[MAGENTA], players[i] -> playerName, colorsCodes[DEFAULT_COLOR]);
                                info = (char*) realloc(info, size + 1);
                                if (!sendData(playerIndex, info)) {
                                    printf("\nError while sending the info!");
                                }
                                free(info);
                            }
                        }
                    }

                } else {
                    // Send the info if is not the game master
                    if (playerIndex == 0) {
                        printColored("\nYou used the KNIFE, but you didn't hurt anybody!", MAGENTA);
                    } else {
                        char* info = (char*) malloc(125);
                        int size = sprintf(info, "%s\nYou used the KNIFE, but you didn't hurt anybody!%s", colorsCodes[MAGENTA], colorsCodes[DEFAULT_COLOR]);
                        info = (char*) realloc(info, size + 1);
                        if (!sendData(playerIndex, info)) {
                            printf("\nError while sending the info!");
                        }
                        free(info);
                    }
                }


                return;            
            
            case 9:
                // Use the TRANQUILLIZER object to increase the mental health by 40
                players[playerIndex] -> mentalHealth += 40;
                players[playerIndex] -> backpack[choice - 1] = EMPTY_SLOT;         
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("%s\nYou used the TRANQUILLIZER, and your mental health has increased to %d!%s",  colorsCodes[MAGENTA], players[playerIndex] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                } else {
                    char* info = (char*) malloc(125);
                    int size = sprintf(info, "%s\nYou used the TRANQUILLIZER, and your mental health has increased to %d!%s",  colorsCodes[MAGENTA], players[playerIndex] -> mentalHealth, colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
                return;   

            default:
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printColored("\nError: please insert a valid input!", RED);
                } else {
                    char* info = (char*) malloc(125);
                    int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }

                if (!playerIndex) {
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
                } else {
                    // Ask to confirm
                    char* tempInfo = (char*) malloc(150);
                    int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                    tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                    if (!sendData(playerIndex, tempInfo)) {
                        printf("\nError while sending the advice!");
                    }
                    
                    free(tempInfo);

                    // Wait the user to continue
                    requestInput(playerIndex);
                }

                break;
        }
    
    } while (TRUE);

    return;
}

static void giveObjects(int playerIndex, int currentTurn) {
    // Check that there's more than one player
    if (playerCount == 1) {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printColored("\nYou can't trade with yourself!", YELLOW);
        } else {
            char* info = (char*) malloc(125);
            int size = sprintf(info, "%s\nYou can't trade with yourself!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }
        return;
    }

    do {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            // Regex to clear the terminal.
            printf("\e[1;1H\e[2J");
            printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            printColored("\n------------- GIVABLE OBJECTS -------------\n", MAGENTA);

        } else {
            char* info = (char*) malloc(250);
            int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- GIVABLE OBJECTS -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }

        // Show the objects that can be traded
        for (int i = 0; i < 4; i++) {
            if ((players[playerIndex] -> backpack[i] != EMPTY_SLOT)) {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("\n%d) Give the %s", i + 1, objectsNames[players[playerIndex] -> backpack[i]]);
                } else {
                    char* info = (char*) malloc(250);
                    int size = sprintf(info, "\n%d) Give the %s", i + 1, objectsNames[players[playerIndex] -> backpack[i]]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
            }
        }

        int choice;

        if (!playerIndex) {        
            // Add an option to exit the menu
            printf("\n5) Exit the menu.");
            printf("\nChoose from the option above: ");
            scanf("%d", &choice);
        } else {
            // Ask to confirm
            if (!sendData(playerIndex, "\n5) Exit the menu.\nChoose from the option above: ")) {
                printf("\nError while sending the advice!");
            }

            // Wait the user input
            char* userInput = requestInput(playerIndex);

            choice = atoi(userInput);

            free(userInput);
        }


        // Check if the input is valid
        if ((1 > choice) || (choice > 5)) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printColored("\nError: please insert a valid input!", RED);
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
            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
                
                // Ask to confirm
                char* tempInfo = (char*) malloc(150);
                int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                if (!sendData(playerIndex, tempInfo)) {
                    printf("\nError while sending the advice!");
                }

                free(tempInfo);

                // Wait the user to continue
                requestInput(playerIndex);
            }

            continue;
        } 
        
        unsigned char selectedObject = players[playerIndex] -> backpack[choice  - 1];

        if (choice == 5) {
            return;
        } else {
            // Check that the selected object is valid
            if (selectedObject == EMPTY_SLOT) {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printColored("\nError: please insert a valid input!", RED);

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

                } else {
                    char* info = (char*) malloc(125);
                    int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);

                    // Ask to confirm
                    char* tempInfo = (char*) malloc(150);
                    int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                    tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                    if (!sendData(playerIndex, tempInfo)) {
                        printf("\nError while sending the advice!");
                    }

                    free(tempInfo);

                    // Wait the user to continue
                    requestInput(playerIndex);
                }

                continue;
            }

            ZoneType currentZone = players[playerIndex] -> position -> zone;

            do {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    // Regex to clear the terminal.
                    printf("\e[1;1H\e[2J");
                    printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
                    printColored("\n------------- AVAILABLE PLAYERS -------------\n", MAGENTA);

                } else {
                    char* info = (char*) malloc(350);
                    int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- AVAILABLE PLAYERS -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }

                unsigned char availablesPlayers[] = {7, 7, 7, 7};
                for (int i = 0; i < playerCount; i++) {
                    // Check if the player has been eliminated
                    if (players[i] == NULL) {
                        continue;
                    }

                    if ((i != playerIndex) && ((players[i] -> position -> zone) == currentZone)) {
                        // Send the info if is not the game master
                        if (playerIndex == 0) {
                            printf("\n%d) Give the %s to %s;", i + 1, objectsNames[selectedObject], players[i] -> playerName);
                        } else {
                            char* info = (char*) malloc(250);
                            int size = sprintf(info, "\n%d) Give the %s to %s;", i + 1, objectsNames[selectedObject], players[i] -> playerName);
                            info = (char*) realloc(info, size + 1);
                            if (!sendData(playerIndex, info)) {
                                printf("\nError while sending the info!");
                            }
                            free(info);
                        }
                        availablesPlayers[i] = i;
                    }
                }

                int option;

                if (!playerIndex) {        
                    // Add an option to exit the menu
                    printf("\n5) Exit the menu.");
                    printf("\nChoose from the option above: ");                    
                    scanf("%d", &option);

                } else {
                    // Ask to confirm
                    if (!sendData(playerIndex, "\n5) Exit the menu.\nChoose from the option above: ")) {
                        printf("\nError while sending the advice!");
                    }
                    // Wait the user input
                    char* userInput = requestInput(playerIndex);

                    option = atoi(userInput);

                    free(userInput);
                }

                // Check if the input is valid
                if ((1 > option) || (option > 5)) {
                    // Send the info if is not the game master
                    if (playerIndex == 0) {
                        printColored("\nError: please insert a valid input!", RED);
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
                    } else {
                        char* info = (char*) malloc(125);
                        int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                        info = (char*) realloc(info, size + 1);
                        if (!sendData(playerIndex, info)) {
                            printf("\nError while sending the info!");
                        }
                        free(info);

                        // Ask to confirm
                        char* tempInfo = (char*) malloc(150);
                        int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                        tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                        if (!sendData(playerIndex, tempInfo)) {
                            printf("\nError while sending the advice!");
                        }

                        free(tempInfo);

                        // Wait the user to continue
                        requestInput(playerIndex);
                    }

                    continue;
                }
                
                unsigned char selectedPlayer = availablesPlayers[option - 1];
                
                if (option == 5) {
                    break;
                } else if (selectedPlayer != 7) {
                    for (int index = 0; index < 4; index++) {
                        if ((players[selectedPlayer] -> backpack[index]) == EMPTY_SLOT) {
                            players[selectedPlayer] -> backpack[index] = selectedObject;
                            players[playerIndex] -> backpack[choice  - 1] = EMPTY_SLOT;
                            
                            // Send the info if is not the game master
                            if (playerIndex == 0) {
                                printf("%s\nYou gave the %s to %s%s", colorsCodes[MAGENTA], objectsNames[selectedObject], players[selectedPlayer] -> playerName, colorsCodes[DEFAULT_COLOR]);
                            } else {
                                char* info = (char*) malloc(250);
                                int size = sprintf(info, "%s\nYou gave the %s to %s%s", colorsCodes[MAGENTA], objectsNames[selectedObject], players[selectedPlayer] -> playerName, colorsCodes[DEFAULT_COLOR]);
                                info = (char*) realloc(info, size + 1);
                                if (!sendData(playerIndex, info)) {
                                    printf("\nError while sending the info!");
                                }
                                free(info);
                            }
                            return;
                        }
                    }
                    
                    // Send the info if is not the game master
                    if (playerIndex == 0) {
                        printColored("\nThe selected player hasn't empty slots!", YELLOW);

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

                    } else {
                        char* info = (char*) malloc(125);
                        int size = sprintf(info, "%s\nThe selected player hasn't empty slots!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                        info = (char*) realloc(info, size + 1);
                        if (!sendData(playerIndex, info)) {
                            printf("\nError while sending the info!");
                        }
                        free(info);

                        // Ask to confirm
                        char* tempInfo = (char*) malloc(150);
                        int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                        tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                        if (!sendData(playerIndex, tempInfo)) {
                            printf("\nError while sending the advice!");
                        }

                        free(tempInfo);

                        // Wait the user to continue
                        requestInput(playerIndex);
                    }

                } else {
                    // Send the info if is not the game master
                    if (playerIndex == 0) {
                        printColored("\nError: please insert a valid input!", RED);

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

                    } else {
                        char* info = (char*) malloc(125);
                        int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                        info = (char*) realloc(info, size + 1);
                        if (!sendData(playerIndex, info)) {
                            printf("\nError while sending the info!");
                        }
                        free(info);

                        // Ask to confirm
                        char* tempInfo = (char*) malloc(150);
                        int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                        tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                        if (!sendData(playerIndex, tempInfo)) {
                            printf("\nError while sending the advice!");
                        }

                        free(tempInfo);

                        // Wait the user to continue
                        requestInput(playerIndex);
                    }
                }

            } while (TRUE);
        }

    } while(TRUE);

    return;
}

static void removeObject(int playerIndex, int currentTurn) {
    do {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            // Regex to clear the terminal.
            printf("\e[1;1H\e[2J");
            printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            printColored("\n------------- REMOVABLE OBJECTS -------------\n", MAGENTA);

        } else {
            char* info = (char*) malloc(250);
            int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- REMOVABLE OBJECTS -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }

        // Show the objects that can be removed
        for (int i = 0; i < 4; i++) {
            if ((players[playerIndex] -> backpack[i] != EMPTY_SLOT)) {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("\n%d) Remove the %s", i + 1, objectsNames[players[playerIndex] -> backpack[i]]);
                } else {
                    char* info = (char*) malloc(250);
                    int size = sprintf(info, "\n%d) Remove the %s", i + 1, objectsNames[players[playerIndex] -> backpack[i]]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
            }
        }

        int choice;

        if (!playerIndex) {        
            // Add an option to exit the menu
            printf("\n5) Exit the menu.");
            printf("\nChoose from the option above: ");            
            scanf("%d", &choice);
        } else {
            // Ask to confirm
            if (!sendData(playerIndex, "\n5) Exit the menu.\nChoose from the option above: ")) {
                printf("\nError while sending the advice!");
            }

            // Wait the user input
            char* userInput = requestInput(playerIndex);

            choice = atoi(userInput);

            free(userInput);
        }

        // Check if the input is valid
        if ((1 > choice) || (choice > 5)) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printColored("\nError: please insert a valid input!", RED);

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

            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);

                // Ask to confirm
                char* tempInfo = (char*) malloc(150);
                int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                if (!sendData(playerIndex, tempInfo)) {
                    printf("\nError while sending the advice!");
                }

                free(tempInfo);

                // Wait the user to continue
                requestInput(playerIndex);
            }
            continue;
        } 
        
        unsigned char selectedObject = players[playerIndex] -> backpack[choice  - 1];

        if (choice == 5) {
            return;
        } else {
            // Check that the selected object is valid
            if (selectedObject == EMPTY_SLOT) {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printColored("\nError: please insert a valid input!", RED);

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

                } else {
                    char* info = (char*) malloc(125);
                    int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);

                    // Ask to confirm
                    char* tempInfo = (char*) malloc(150);
                    int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                    tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                    if (!sendData(playerIndex, tempInfo)) {
                        printf("\nError while sending the advice!");
                    }

                    free(tempInfo);

                    // Wait the user to continue
                    requestInput(playerIndex);
                }
                continue;
            }
            
            // Remove the object
            players[playerIndex] -> backpack[choice - 1] = EMPTY_SLOT;
            
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("%s\nYou have removed the %s from the backpack!%s", colorsCodes[MAGENTA], objectsNames[players[playerIndex] -> backpack[choice - 1]], colorsCodes[DEFAULT_COLOR]);
            } else {
                char* info = (char*) malloc(250);
                int size = sprintf(info, "%s\nYou have removed the %s from the backpack!%s", colorsCodes[MAGENTA], objectsNames[players[playerIndex] -> backpack[choice - 1]], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
            return;
        }

    } while (TRUE);
    
    return;
}

static void reorginizeBackpack(int playerIndex, int currentTurn) {
    do {
        int emptySlots = 0;
        
        // Send the info if is not the game master
        if (playerIndex == 0) {
            // Regex to clear the terminal.
            printf("\e[1;1H\e[2J");
            printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            printColored("\n------------- BACKPACK SLOTS -------------\n", MAGENTA);

        } else {
            char* info = (char*) malloc(250);
            int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- BACKPACK SLOTS -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }

        for (int i = 0; i < 4; i++) {
            unsigned char slot = players[playerIndex] -> backpack[i];

            // Count the number of empty slots
            if (slot == EMPTY_SLOT) {
                emptySlots++;
            }
            
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("\n%d) %s;", i + 1, objectsNames[slot]);
            } else {
                char* info = (char*) malloc(250);
                int size = sprintf(info, "\n%d) %s;", i + 1, objectsNames[slot]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
        }

        // Check that at least one slot is used
        if (emptySlots == 4) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printColored("\nAll the slots are empty, there's nothing to reorginize!", YELLOW);
            } else {
                char* info = (char*) malloc(250);
                int size = sprintf(info, "%s\nAll the slots are empty, there's nothing to reorginize!%s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
            return;
        }

        int choice;

        if (!playerIndex) {        
            // Add an option to exit the menu
            printf("\n5) Exit the menu.");
            printf("\nInsert the number of the slot to swap: ");            
            scanf("%d", &choice);
        } else {
            // Ask to confirm
            if (!sendData(playerIndex, "\n5) Exit the menu.\nInsert the number of the slot to swap: ")) {
                printf("\nError while sending the advice!");
            }

            // Wait the user input
            char* userInput = requestInput(playerIndex);

            choice = atoi(userInput);

            free(userInput);
        }


        // Check for invalid input
        if ((1 > choice) || (choice > 5)) {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printColored("\nError: please insert a valid input!", RED);

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

            } else {
                char* info = (char*) malloc(125);
                int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);

                // Ask to confirm
                char* tempInfo = (char*) malloc(150);
                int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                if (!sendData(playerIndex, tempInfo)) {
                    printf("\nError while sending the advice!");
                }

                free(tempInfo);

                // Wait the user to continue
                requestInput(playerIndex);
            }

            continue;
        } else if (choice == 5) {
            return;
        }

        unsigned char slotToSwap = players[playerIndex] -> backpack[choice - 1];

        // Ask the player which slot to swap with the one selected before
        do {
            // Send the info if is not the game master
            if (playerIndex == 0) {
                // Regex to clear the terminal.
                printf("\e[1;1H\e[2J");
                printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
                printColored("\n------------- BACKPACK SLOTS -------------\n", MAGENTA);

            } else {
                char* info = (char*) malloc(250);
                int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- BACKPACK SLOTS -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
            
            // Show all the object in the backpack except the one selected before
            for (int i = 0; i < 4; i++) {
                if (i == (choice - 1)) {
                    continue;
                }

                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printf("\n%d) %s;", i + 1, objectsNames[players[playerIndex] -> backpack[i]]);
                } else {
                    char* info = (char*) malloc(250);
                    int size = sprintf(info, "\n%d) %s;", i + 1, objectsNames[players[playerIndex] -> backpack[i]]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);
                }
            }

            int option;

            if (!playerIndex) {        
                // Add an option to exit the menu
                printf("\n5) Exit the menu.");
                printf("\nInsert the number of the slot to swap with the %s: ", objectsNames[slotToSwap]);
                scanf("%d", &option);
            } else {
                // Ask to confirm
                char* subMenuInfo = (char*) malloc(150);
                int infoSize = sprintf(subMenuInfo, "\n5) Exit the menu.\nInsert the number of the slot to swap with the %s: ", objectsNames[slotToSwap]);
                subMenuInfo = (char*) realloc(subMenuInfo, infoSize + 1);
                if (!sendData(playerIndex, subMenuInfo)) {
                    printf("\nError while sending the advice!");
                }

                free(subMenuInfo);

                // Wait the user input
                char* userInput = requestInput(playerIndex);

                option = atoi(userInput);

                free(userInput);
            }

            // Check for invalid input
            if ((1 > option) || (option > 5) || (option == choice)) {
                // Send the info if is not the game master
                if (playerIndex == 0) {
                    printColored("\nError: please insert a valid input!", RED);

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

                } else {
                    char* info = (char*) malloc(125);
                    int size = sprintf(info, "%s\nError: please insert a valid input!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                    info = (char*) realloc(info, size + 1);
                    if (!sendData(playerIndex, info)) {
                        printf("\nError while sending the info!");
                    }
                    free(info);

                    // Ask to confirm
                    char* tempInfo = (char*) malloc(150);
                    int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
                    tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
                    if (!sendData(playerIndex, tempInfo)) {
                        printf("\nError while sending the advice!");
                    }

                    free(tempInfo);

                    // Wait the user to continue
                    requestInput(playerIndex);
                }
                continue;
            } else if (option == 5) {
                break;
            }

            // Swap the selected slots
            unsigned char selectedSlot = players[playerIndex] -> backpack[option - 1]; 
            players[playerIndex] -> backpack[choice - 1] = selectedSlot;
            players[playerIndex] -> backpack[option - 1] = slotToSwap;

                        
            // Send the info if is not the game master
            if (playerIndex == 0) {
                printf("%s\nSwapped the %s with the %s!%s", colorsCodes[MAGENTA], objectsNames[slotToSwap], objectsNames[selectedSlot], colorsCodes[DEFAULT_COLOR]);
            } else {
                char* info = (char*) malloc(250);
                int size = sprintf(info, "%s\nSwapped the %s with the %s!%s", colorsCodes[MAGENTA], objectsNames[slotToSwap], objectsNames[selectedSlot], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(playerIndex, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }
            return;

        } while (TRUE);

    } while (TRUE);

    return;
}

static int randomNumber(int range) {
    // Check if the current time has been already initialized
    if (currentTime == 0) {
        // Initialize the random number generator using the current time
        srand((unsigned) time(&currentTime));
    }

    // Generate some numbers to increase the randomness
    for (int i = 0; i < 10; i++) {
        rand();
    }

    // Generate a random number and set it into the given range
    return (rand() % range);
}

static void generateTurns() {
    // Check if the turns array has been already used
    if (turns != NULL) {
        free(turns);
        turns = NULL;
    }

    // Allocate the memory for the turns array
    turns = (int*) calloc(playerCount, sizeof(int));

    // Generate the players' turns
    for (int i = 0; i < playerCount; i++) {
        // Generate a random number
        int randomNum = randomNumber(playerCount);
        
        for (int l = 0; l < i; l++) {
            // If the generated number is already in the array, regenerate a new one and check it
            if (turns[l] == randomNum) {
                // Reset the counter of the for loop
                l = -1;

                // Generate a new random number
                randomNum = randomNumber(playerCount);
            }
        }
        
        turns[i] = randomNum;
    }

    return;
}

static void checkGameStatus() {
    // If all the three different type of evidence has been collected, then the players win
    if ((caravanEvidence[0] != NO_EVIDENCE) && (caravanEvidence[1] != NO_EVIDENCE) && (caravanEvidence[2] != NO_EVIDENCE)) {
        gameState = WIN;
        return;
    }

    int playersEliminated = 0;
    for (int i = 0; i < playerCount; i++) {
        // If the mental health of the player is equal or less than 0, then eliminate the player
        if ((players[i] -> mentalHealth) <= 0) {
            free(players[i]);
            players[i] = NULL;

            if (i == 0) {
                printColored("\nYou have been eliminated because your mental health is less than 0!", RED);
                
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

            } else {
                char* info = (char*) malloc(150);
                int size = sprintf(info, "%s\nYou have been eliminated because your mental health is less than 0!%s", colorsCodes[RED], colorsCodes[DEFAULT_COLOR]);
                info = (char*) realloc(info, size + 1);
                if (!sendData(i, info)) {
                    printf("\nError while sending the info!");
                }
                free(info);
            }

            // Ask to confirm
            char* tempInfo = (char*) malloc(150);
            int tempInfoSize = sprintf(tempInfo, "\n%sPress ENTER to continue: %s", colorsCodes[YELLOW], colorsCodes[DEFAULT_COLOR]);
            tempInfo = (char*) realloc(tempInfo, tempInfoSize + 1);
            if (!sendData(i, tempInfo)) {
                printf("\nError while sending the advice!");
            }

            free(tempInfo);

            // Wait the user to continue
            requestInput(i);
        
        }

        // Increment the counter for each player that has been eliminated
        if (players[i] == NULL) {
            playersEliminated++;
        }
    }

    // If every player has been eliminated, then game over
    if (playersEliminated == playerCount) {
        gameState = GAME_OVER;
        return;
    }

    return;
}

static char* printObjectsInfo(unsigned char objects[]) {
    unsigned char duplicates[] = {0, 0, 0, 0};
    int duplicateCount = 0;
    int isDuplicated;
    
    for (int index = 0; index < 4; index++) {
        // Skip the object if it's not valid
        if ((4 > objects[index]) && (objects[index] > 9)) {
            continue;
        }

        // Check for duplicates
        isDuplicated = 0;
        for (int i = 0; i < duplicateCount; i++) {
            if (objects[index] == duplicates[i]) {
                isDuplicated = 1;
                break;
            }
        }

        // If the current object is not a duplicate than print its effect
        if (!isDuplicated) {
            // Add the object to the duplicates
            duplicates[duplicateCount] = objects[index];
            duplicateCount++;

            // Show the info of the object
            switch (objects[index]) {
                case 4:
                    return "\n- Use the SEDATIVE to increase the mental health by 40 points";

                case 5:
                    return "\n- Use the SALT to prevent a decrement of the mental health, caused by the ghost";

                case 6:
                    return "\n- Use the ADRENALINE to go to the next zone and obtain an extra turn";

                case 7:
                    return "\n- Use the HUNDRED_DOLLAR to buy a TRANQUILLIZER or SALT";

                case 8:
                    return "\n- Use the KNIFE and if the mental health is under 30 kill all the players in the same zone as the current player";

                case 9:
                    return "\n- Use the TRANQUILLIZER to increase the mental health by 40 points";
            }
        }
    }

    return " ";
}

static void printEvidenceCollected(int playerIndex, int currentTurn) {
    // Send the info if is not the game master
    if (playerIndex == 0) {
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");
        printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
        printColored("\n------------- EVIDENCE COLLECTED IN THE CARAVAN -------------\n", MAGENTA);

    } else {
        char* info = (char*) malloc(250);
        int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- EVIDENCE COLLECTED IN THE CARAVAN -------------\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }


    for (int i = 0; i < 3; i++) {
        // Send the info if is not the game master
        if (playerIndex == 0) {
            printf("\n%d) %s", i + 1, objectsNames[caravanEvidence[i]]);
        } else {
            char* info = (char*) malloc(250);
            int size = sprintf(info, "\n%d) %s", i + 1, objectsNames[caravanEvidence[i]]);
            info = (char*) realloc(info, size + 1);
            if (!sendData(playerIndex, info)) {
                printf("\nError while sending the info!");
            }
            free(info);
        }
    }
    
    return;
}

static void printGhostInfo(int playerIndex, int currentTurn) {
    // Send the info if is not the game master
    if (playerIndex == 0) {
        // Regex to clear the terminal.
        printf("\e[1;1H\e[2J");
        printf("\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n%s", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR]);
        printColored("\n------------- GHOST INFO -------------", MAGENTA);
        printf("\nGhost position: %s", zoneTypeNames[ghostPosition]);
        printf("\nGhost appeareance probability: %d%%", ghostAppearance);        

    } else {
        char* info = (char*) malloc(250);
        int size = sprintf(info, "\e[1;1H\e[2J\n%sROUND: %d - TURN: %d - CURRENTLY PLAYING: %s\n\n------------- GHOST INFO -------------\n%s\nGhost position: %s\nGhost appeareance probability: %d%%", colorsCodes[MAGENTA], (roundCount + 1), (currentTurn + 1), players[playerIndex] -> playerName, colorsCodes[DEFAULT_COLOR], zoneTypeNames[ghostPosition], ghostAppearance);
        info = (char*) realloc(info, size + 1);
        if (!sendData(playerIndex, info)) {
            printf("\nError while sending the info!");
        }
        free(info);
    }

    return;
}

static void printColored(char* str, ColorType color) {
    printf("%s%s%s", colorsCodes[color], str, colorsCodes[DEFAULT_COLOR]);
    return;
}

static char* printAdvices(int playerIndex) {
    Player* player = players[playerIndex];
    char* temp = (char*) malloc(375);
    int currentLen = 0;

    // Check if the player have an evidence to deposit
    for (int i = 0; i < 4; i++) {
        unsigned char slot = player -> backpack[i];
        if ((slot >= 11) && (slot != EMPTY_SLOT)) {
            currentLen = sprintf(temp, "ADVICE: Deposit the evidence in the caravan! (Type 1)");
            temp = (char*) realloc(temp, currentLen + 1);            
            return temp;
        }
    }    
    
    // Check if there's an evidence to pick
    for (int i = 0; i < 4; i++) {
        unsigned char slot = (player -> backpack[i]) + 10;
        unsigned char currentEvidence = (player -> position -> evidence);
        if ((currentEvidence == slot) && (slot != NO_EVIDENCE)) {
            currentLen = sprintf(temp, "ADVICE: Pick the evidence from the current zone! (Type 3)");
            temp = (char*) realloc(temp, currentLen + 1);
            return temp;
        }
    }    
    
    // Check if the player have an object to use
    for (int i = 0; i < 4; i++) {
        unsigned char slot = player -> backpack[i];
        if ((3 < slot) && (slot < 10)) {
            currentLen = sprintf(temp, "ADVICE: Use an object from the backpack! (Type 5)");
            temp = (char*) realloc(temp, currentLen + 1);
            return temp;
        }
    }    
    
    // Check if there's an object to pick
    for (int i = 0; i < 4; i++) {
        unsigned char currentZoneObject = player -> position -> zoneObject;
        if ((0 < currentZoneObject) && (currentZoneObject < 10)) {
            currentLen = sprintf(temp, "ADVICE: Pick the object from the current zone! (Type 4)");
            temp = (char*) realloc(temp, currentLen + 1);
            return temp;
        }
    }    
    
    // Before checking if is better skip the turn, check if there's more than one player
    if (playerCount > 1) {
        // Check if there's a player in the same zone that has the object to pick the evidence from the current zone
        for (int index = 0; index < playerCount; index++) {
            // Don't evaluate the current player
            if (index == playerIndex) {
                continue;
            }

            ZoneType currentEvaluatedZone = player -> position -> zone;
            if (currentEvaluatedZone != (players[index] -> position -> zone)) {
                continue;
            }

            unsigned char currentZoneEvidence = (player -> position -> evidence);

            for (int i = 0; i < 4; i++) {
                // Add 10 to the current player object to verify if the objects match the corresponding evidence
                unsigned char playerSlot = (players[index] -> backpack[i]) + 10;

                if ((playerSlot == currentZoneEvidence) && (playerSlot != NO_EVIDENCE)) {
                    currentLen = sprintf(temp, "ADVICE: Skip the turn, because %s has the object to pick the evidence from the current zone! (Type 6)", players[index] -> playerName);
                    temp = (char*) realloc(temp, currentLen + 1);
                    return temp;
                }
            }
        }
    }

    // If there's nothing to do, advise to go to the next zone
    currentLen = sprintf(temp, "ADVICE: Go to the next zone! (Type 2)");
    temp = (char*) realloc(temp, currentLen + 1);
    return temp;

}

/* END OF DEFINITIONS */
