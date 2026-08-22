#ifndef ROOM_LOADER_H
#define ROOM_LOADER_H

#include <filesystem>

struct Room;

namespace room_loader
{

/**
 * @brief Load a Room from a text file authored under assets/rooms/.
 *
 * @param roomID Unique identifier assigned to the loaded room.
 * @param path   Path to the room file.
 * @return A fully populated Room ready to be added to a Level.
 */
Room loadRoom(int roomID, const std::filesystem::path& path);

}  // namespace room_loader

#endif
