#include "preload/room_loader.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "objects/tiles/tile.h"
#include "objects/tiles/tile_type.h"

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

// parse the '@key: value' header block, leaving `in` positioned at the start
// of the grid. blank lines before the grid are permitted so authors can
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
      // rewind — this line belongs to the grid.
      in.clear();
      in.seekg(gridStart);
      return;
    }

    // parse "@key: value".
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
      room.setName(value);
    }
    // other keys (levels, author, ...) are parsed by the library layer or
    // silently ignored here for forward compatibility.

    gridStart = in.tellg();
  }
}

// parse the ASCII grid following the header, populating tiles, doors, and
// spawn points. throws if the row count doesn't match Room::HEIGHT.
void parseRoomGrid(std::ifstream& in, Room& room,
                   const std::filesystem::path& path,
                   std::vector<Coordinate>& enemySpawns,
                   std::vector<Coordinate>& lootSpawns,
                   std::vector<Coordinate>& itemSpawns)
{
  std::string line;
  int y = 0;
  while (std::getline(in, line))
  {
    // strip trailing CR so CRLF-terminated files (common on Windows editors)
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
    // pad short lines with spaces (Void) but reject over-long lines to catch
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
      room.setTile(Coordinate(x, y), Tile(type, Coordinate(x, y)));
      if (type == TileType::Door)
      {
        DoorNumber label = c - '0';
        if (room.getDoors().find(label) != room.getDoors().end())
        {
          throw std::runtime_error("Duplicate door label '" +
                                   std::string(1, c) + "' in " + path.string());
        }
        room.addDoor(label, Coordinate{x, y});
      }
      switch (charToSpawnKind(c))
      {
        case SpawnKind::Enemy:
          enemySpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::LootOrItem:
          lootSpawns.push_back(Coordinate(x, y));
          itemSpawns.push_back(Coordinate(x, y));
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

Coordinate doorAt(const Room& room, DoorNumber number)
{
  const auto& doors = room.getDoors();
  auto doorEntry = doors.find(number);
  if (doorEntry == doors.end())
  {
    throw std::runtime_error("room " + std::to_string(room.getRoomID()) + " (" +
                             room.getName() + ") has no door " +
                             std::to_string(number));
  }
  return doorEntry->second;
}

Coordinate inwardOfDoor(Coordinate doorPos)
{
  Coordinate inward = doorPos;
  if (doorPos.x == 0)
  {
    inward.x = 1;
  }
  else if (doorPos.x == Room::WIDTH - 1)
  {
    inward.x = Room::WIDTH - 2;
  }
  else if (doorPos.y == 0)
  {
    inward.y = 1;
  }
  else if (doorPos.y == Room::HEIGHT - 1)
  {
    inward.y = Room::HEIGHT - 2;
  }
  return inward;
}

ParsedRoom loadRoom(int roomID, const std::filesystem::path& path)
{
  std::ifstream in(path);
  if (!in)
  {
    throw std::runtime_error("Could not open room file: " + path.string());
  }

  Room room(roomID);
  std::vector<Coordinate> enemySpawns;
  std::vector<Coordinate> lootSpawns;
  std::vector<Coordinate> itemSpawns;
  parseRoomHeader(in, room, path);
  parseRoomGrid(in, room, path, enemySpawns, lootSpawns, itemSpawns);

  for (const auto& [number, doorPos] : room.getDoors())
  {
    Coordinate entry = inwardOfDoor(doorPos);
    room.setTile(entry, Tile(TileType::EntryWay, entry));
  }

  return ParsedRoom{std::move(room), std::move(enemySpawns),
                    std::move(lootSpawns), std::move(itemSpawns)};
}

}  // namespace room_loader
