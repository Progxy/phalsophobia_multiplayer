#pragma once

#ifndef _NETWORK_H
#define _NETWORK_H
#endif

#include "utils.h"

/// @brief Start the game as game master.
/// @return Return the status of the operation.
bool enterGame();

/// @brief Send the generated player data.
/// @param player
/// @return Return the status of the operation.
bool sendPlayerData(Player player);
