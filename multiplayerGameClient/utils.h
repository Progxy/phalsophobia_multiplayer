//NOTE: This file contains all the functions needed to perform utility functions.

#pragma once

#ifndef _UTILS_H
#define _UTILS_H
#endif


#define TRUE 1
#define FALSE 0
#define EMPTY_SLOT 15

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
    int length;
    struct dataReceived* next;
} dataReceived;

/// @brief Set the player and send the generated data to the server.
void setPlayer();

/// @brief Play your turn.
void playTurn();

