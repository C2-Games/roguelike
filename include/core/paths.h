#ifndef PATHS_H
#define PATHS_H

/**
 * @brief Filesystem paths used by the game at runtime.
 *
 * All paths are relative to the working directory (the repo root when
 * launched from there). If the game is ever run from a different CWD or
 * shipped with an installer, this is the one place to change them.
 */
namespace paths {

inline constexpr const char* kRoomsDir = "assets/rooms";
inline constexpr const char* kRoomFileExt = ".txt";

inline constexpr const char* kGameLogPath = "game.log";
inline constexpr const char* kErrorLogPath = "error.log";

}  // namespace paths

#endif
