#ifndef _SERVER_H_
#define _SERVER_H_

#include "utils.h"

typedef enum utilsStatus {SEARCHING, END, INVALID_SOCKET = -1, INVALID_RESPONSE = -1} utilsStatus;

/// @brief Load the server with its basic functions.
/// @return Return the status of the operation.
int loadServer(void);

/// @brief Send the data to the given target.
/// @param clientIndex
/// @param message
/// @param msgLen
/// @return Return the status of the operation.
bool sendData(int clientIndex, char* message);

/// @brief Receive the data from the given target.
/// @param clientSocket
void* receiveData(void* vargp);

/// @brief Get the number of elements in the list of messages received from the server.
/// @return Return the len of the list.
int getDataReceivedLen(void);

/// @brief Retrieve the data sent by the server.
/// @return Return the most recent message received, if there's.
dataReceived getDataReceived(void);

/// @brief Create the server list.
/// @return Return the number of players connected to the server.
int createServerList(void);

/// @brief Close the server socket and deallocate the memory used.
void closeServer(void);

#endif //_SERVER_H_
