//NOTE: This file contains all the functions needed to perform utility functions.
#ifndef _UTILS_H
#define _UTILS_H

#define TRUE 1
#define FALSE 0
#define EMPTY_SLOT 15
#define UNUSED __attribute__((unused))

typedef int bool;

typedef enum TurnStates {PLAYING, FINISHED} TurnStates;
typedef enum ColorType {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, DEFAULT_COLOR} ColorType;
typedef enum PropertyState {INACTIVE, ACTIVE} PropertyState;

typedef struct Player {
    char* playerName;
    PropertyState useAdvices;
} Player;

typedef struct dataReceived {
    char* data;
    struct dataReceived* next;
} dataReceived;

/// @brief Set the player and send the generated data to the server.
void setPlayer(void);

/// @brief Play your turn.
void playTurn(void);

#endif //_UTILS_H
