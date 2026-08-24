#ifndef ROOM_LOADER_H
#define ROOM_LOADER_H

#include <filesystem>

struct Room;

namespace room_loader
{

/**
 * @brief Parse one room file into a Room, including doors and spawn points.
 *
 * @param roomID Unique id to assign the parsed room.
 * @param path Path to the room's .txt template under assets/rooms/.
 * @return The fully parsed room.
 */
Room loadRoom(int roomID, const std::filesystem::path& path);

}  // namespace room_loader

#endif
