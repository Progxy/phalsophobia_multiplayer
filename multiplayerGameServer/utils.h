//NOTE: This file contains all the functions needed to perform utility functions.

#pragma once

#ifndef _UTILS_H
#define _UTILS_H
#endif

#define TRUE 1
#define FALSE 0
#define EMPTY_SLOT 15

typedef int bool;

typedef enum GameStates {UNSET, SET, WIN, GAME_OVER} GameStates;
typedef enum TurnStates {PLAYING, FINISHED} TurnStates;
typedef enum ColorType {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, DEFAULT_COLOR} ColorType;
typedef enum PropertyState {INACTIVE, ACTIVE} PropertyState;

typedef enum GameDifficulties {AMATEUR, INTERMEDIATE, NIGHTMARE, INCREMENT_AMATEUR = 2, INCREMENT_INTERMEDIATE = 5, INCREMENT_NIGHTMARE = 10, DECREMENT_AMATEUR = 15, DECREMENT_INTERMEDIATE = 20, DECREMENT_NIGHTMARE = 30} GameDifficulties;
typedef enum ZoneType {CARAVAN, KITCHEN, LIVING_ROOM, ROOM, BATHROOM, GARAGE, BASEMENT, NO_ZONE} ZoneType;
typedef enum EvidenceType {EMF_EVIDENCE = 11, SPIRIT_BOX_EVIDENCE, CAMERA_EVIDENCE, NO_EVIDENCE} EvidenceType;
typedef enum StarterObjectType {EMF = 1, SPIRIT_BOX, CAMERA, SEDATIVE, SALT} StarterObjectType;
typedef enum ZoneObjectType {ADRENALINE = 6, HUNDRED_DOLLAR, KNIFE, TRANQUILLIZER, NO_OBJECT} ZoneObjectType;

typedef struct MapZone {
    ZoneType zone;
    EvidenceType evidence;
    ZoneObjectType zoneObject;
    struct MapZone* nextZone;
} MapZone;

typedef struct Player {
    char* playerName;
    unsigned char mentalHealth;
    MapZone* position;
    unsigned char backpack[4];
    PropertyState useAdvices;
    PropertyState saltProtection;
} Player;

typedef struct dataReceived {
    char* data;
    int length;
    int clientId;
    struct dataReceived* next;
} dataReceived;

/// @brief Set the game. 
void set(int playerNum);

/// @brief Set the player at the given index.
/// @param playerIndex
/// @param info 
void setPlayers(int playerIndex, char* info);

/// @brief Return the current settings.
/// @param response 
/// @return Return the game settings.
char* showGameSettings();

/// @brief Start the game.
void playGame();

/// @brief Reset the data.
void resetData();

/// @brief Close the game by deallocating all the memory from the heap.
void close();