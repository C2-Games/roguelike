#include "preload/room_loader.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "objects/room/room.h"
#include "objects/room/room_types.h"

namespace
{

std::string trim(std::string s)
{
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

TileType charToRoomTile(char c)
{
  if (c == '#')
  {
    return TileType::Wall;
  }
  if (c == '.')
  {
    return TileType::Floor;
  }
  if (c == 'o')
  {
    return TileType::Pillar;
  }
  if (c == ' ')
  {
    return TileType::Void;
  }
  if (c == 'E' || c == 'L')
  {
    return TileType::Floor;
  }
  if (c >= '0' && c <= '9')
  {
    return TileType::Door;
  }
  std::ostringstream oss;
  oss << "Unrecognized room-file character: '" << c << "' (0x" << std::hex
      << static_cast<int>(static_cast<unsigned char>(c)) << ")";
  throw std::runtime_error(oss.str());
}

enum class SpawnKind : std::uint8_t
{
  None,
  Enemy,
  LootOrItem
};

SpawnKind charToSpawnKind(char c)
{
  switch (c)
  {
    case 'E':
      return SpawnKind::Enemy;
    case 'L':
      return SpawnKind::LootOrItem;
    default:
      return SpawnKind::None;
  }
}

// Parse the '@key: value' header block, leaving `in` positioned at the start
// of the grid. Blank lines before the grid are permitted so authors can
// space out the header visually.
void parseRoomHeader(std::ifstream& in, Room& room,
                     const std::filesystem::path& path)
{
  std::string line;
  std::streampos gridStart = in.tellg();
  while (std::getline(in, line))
  {
    std::string trimmed = trim(line);
    if (trimmed.empty())
    {
      gridStart = in.tellg();
      continue;
    }
    if (trimmed[0] != '@')
    {
      // Rewind — this line belongs to the grid.
      in.clear();
      in.seekg(gridStart);
      return;
    }

    // Parse "@key: value".
    auto colon = trimmed.find(':');
    if (colon == std::string::npos)
    {
      throw std::runtime_error("Malformed header (no colon) in " +
                               path.string() + ": " + trimmed);
    }
    std::string key = trim(trimmed.substr(1, colon - 1));
    std::string value = trim(trimmed.substr(colon + 1));

    if (key == "name")
    {
      room.name = value;
    }
    // Other keys (levels, author, ...) are parsed by the library layer or
    // silently ignored here for forward compatibility.

    gridStart = in.tellg();
  }
}

// Parse the ASCII grid following the header, populating tiles, doors, and
// spawn points. Throws if the row count doesn't match Room::HEIGHT.
void parseRoomGrid(std::ifstream& in, Room& room,
                   const std::filesystem::path& path)
{
  std::string line;
  int y = 0;
  while (std::getline(in, line))
  {
    // Strip trailing CR so CRLF-terminated files (common on Windows editors)
    // parse the same as LF.
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }

    if (y >= Room::HEIGHT)
    {
      throw std::runtime_error("Too many grid rows in " + path.string() +
                               " (expected " + std::to_string(Room::HEIGHT) +
                               ")");
    }
    // Pad short lines with spaces (Void) but reject over-long lines to catch
    // authoring mistakes.
    if (static_cast<int>(line.size()) > Room::WIDTH)
    {
      throw std::runtime_error(
          "Row " + std::to_string(y) + " in " + path.string() +
          " is too wide: " + std::to_string(line.size()) + " chars (expected " +
          std::to_string(Room::WIDTH) + ")");
    }
    if (static_cast<int>(line.size()) < Room::WIDTH)
    {
      line.append(Room::WIDTH - line.size(), ' ');
    }

    for (int x = 0; x < Room::WIDTH; ++x)
    {
      char c = line[x];
      TileType type = charToRoomTile(c);
      room.tiles[x][y] = Tile(type, Coordinate(x, y));
      if (type == TileType::Door)
      {
        DoorNumber label = c - '0';
        const bool inserted =
            room.doors.emplace(label, Coordinate{x, y}).second;
        if (!inserted)
        {
          throw std::runtime_error("Duplicate door label '" +
                                   std::string(1, c) + "' in " + path.string());
        }
      }
      switch (charToSpawnKind(c))
      {
        case SpawnKind::Enemy:
          room.enemySpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::LootOrItem:
          room.lootSpawns.push_back(Coordinate(x, y));
          room.itemSpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::None:
          break;
      }
    }
    ++y;
  }

  if (y != Room::HEIGHT)
  {
    throw std::runtime_error("Not enough grid rows in " + path.string() +
                             ": got " + std::to_string(y) + ", expected " +
                             std::to_string(Room::HEIGHT));
  }
}

}  // namespace

namespace room_loader
{

Room loadRoom(int roomID, const std::filesystem::path& path)
{
  std::ifstream in(path);
  if (!in)
  {
    throw std::runtime_error("Could not open room file: " + path.string());
  }

  Room room(roomID);
  parseRoomHeader(in, room, path);
  parseRoomGrid(in, room, path);

  for (const auto& [number, doorPos] : room.doors)
  {
    Coordinate entry = Room::inwardOfDoor(doorPos);
    room.tiles[entry.x][entry.y] = Tile(TileType::EntryWay, entry);
  }

  return room;
}

}  // namespace room_loader
