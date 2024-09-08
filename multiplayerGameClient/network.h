#ifndef _NETWORK_H
#define _NETWORK_H

#include "utils.h"

/// @brief Start the game as game master.
/// @return Return the status of the operation.
bool enterGame(void);

/// @brief Send the generated player data.
/// @param player
/// @return Return the status of the operation.
bool sendPlayerData(Player player);

#endif //_NETWORK_H
