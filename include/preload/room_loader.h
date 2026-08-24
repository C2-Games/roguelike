#ifndef ROOM_LOADER_H
#define ROOM_LOADER_H

#include <filesystem>
#include <vector>

#include "objects/coordinate.h"
#include "objects/room/room.h"

namespace room_loader
{

// one room's static geometry plus the authored spawn points found while
// parsing its grid.
struct ParsedRoom
{
  Room room;
  std::vector<Coordinate> enemySpawns;
  std::vector<Coordinate> lootSpawns;
  std::vector<Coordinate> itemSpawns;
};

/**
 * @brief Parse one room file into a Room, including doors and spawn points.
 *
 * @param roomID Unique id to assign the parsed room.
 * @param path Path to the room's .txt template under assets/rooms/.
 * @return The fully parsed room and its spawn points.
 */
ParsedRoom loadRoom(int roomID, const std::filesystem::path& path);

/**
 * @brief Look up a room's door position by its authored label.
 *
 * @param room Room to query.
 * @param number Door's authored label from the room's .txt grid.
 * @return The door tile's grid position.
 */
Coordinate doorAt(const Room& room, DoorNumber number);

/**
 * @brief Find the tile one step inward from a door position.
 *
 * @param doorPos Door tile's grid position, assumed to lie on the room's
 *                boundary.
 * @return The adjacent tile position, one step toward room interior.
 */
Coordinate inwardOfDoor(Coordinate doorPos);

}  // namespace room_loader

#endif
