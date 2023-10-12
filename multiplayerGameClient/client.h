//NOTE: This the core client header, where are contained all the functions necessary to communicate between users.

#pragma once

#ifndef _CLIENT_H
#define _CLIENT_H
#endif

#include <sys/socket.h>
#include "utils.h"

/// @brief Init the client with its basic functions.
/// @return Return the status of the operation.
bool initClient();

/// @brief Send the data to the given target.
/// @param message 
/// @param msgLen 
/// @return Return the status of the operation.
bool sendData(char* message);

/// @brief Receive the data from the given target.
void* receiveData();

/// @brief Retrieve the data sent by the server.
/// @return Return the most recent message received, if there's.
char* getDataReceived();

/// @brief Close the client socket and deallocate the memory used.
void closeClient();
