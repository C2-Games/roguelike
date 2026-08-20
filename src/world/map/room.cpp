#include "world/map/room.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "entities/fov.h"

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
  if (c == '#') return TileType::Wall;
  if (c == '.') return TileType::Floor;
  if (c == 'o') return TileType::Pillar;
  if (c == ' ') return TileType::Void;
  if (c == 'E' || c == 'L') return TileType::Floor;
  if (c >= '0' && c <= '9') return TileType::Door;
  std::ostringstream oss;
  oss << "Unrecognized room-file character: '" << c << "' (0x" << std::hex
      << static_cast<int>(static_cast<unsigned char>(c)) << ")";
  throw std::runtime_error(oss.str());
}

enum class SpawnKind
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

}  // namespace

Room::Room(int id) : roomID(id), tiles(WIDTH, std::vector<Tile>(HEIGHT)) {}

void Room::updateVisibility(Coordinate origin, const FOV& fov)
{
  clearVisible();
  for (const Coordinate& pos : fov.absoluteFOV(origin))
  {
    reveal(pos.x, pos.y);
  }
}

bool Room::updateVisibilityDelta(Coordinate previousOrigin, Coordinate origin,
                                 const FOV& fov)
{
  Coordinate dir = origin - previousOrigin;
  const std::vector<Coordinate>* leaving = fov.leavingOffsets(dir);
  const std::vector<Coordinate>* entering =
      fov.leavingOffsets(Coordinate(-dir.x, -dir.y));
  if (leaving == nullptr || entering == nullptr) return false;

  for (const Coordinate& offset : *leaving)
  {
    Coordinate pos = previousOrigin + offset;
    if (pos.x < 0 || pos.x >= WIDTH || pos.y < 0 || pos.y >= HEIGHT) continue;
    // No bounds-checked equivalent of reveal() exists for clearing a single
    // tile, so this open-codes the same bounds check reveal() does.
    tiles[pos.x][pos.y].clearVisible();
  }

  for (const Coordinate& offset : *entering)
  {
    Coordinate pos = origin + offset;
    reveal(pos.x, pos.y);
  }

  return true;
}

void Room::updateVisibility(Coordinate previousOrigin, Coordinate origin,
                            const FOV& fov)
{
  if (!updateVisibilityDelta(previousOrigin, origin, fov))
  {
    updateVisibility(origin, fov);
  }
}

void Room::clearVisible()
{
  for (auto& col : tiles)
  {
    for (auto& tile : col)
    {
      tile.clearVisible();
    }
  }
}

bool Room::isVisible(int x, int y) const
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
  return tiles[x][y].isVisible();
}

bool Room::isExplored(int x, int y) const
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
  return tiles[x][y].isExplored();
}

void Room::reveal(int x, int y)
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  tiles[x][y].reveal();
}

Room Room::loadFromFile(int roomID, const std::filesystem::path& path)
{
  std::ifstream in(path);
  if (!in)
  {
    throw std::runtime_error("Could not open room file: " + path.string());
  }

  Room room(roomID);

  // --- Parse header ---
  // Header lines start with '@'. Grid begins at the first non-'@', non-empty
  // line. Blank lines before the grid are permitted so authors can space out
  // the header visually.
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
      break;
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

  // --- Parse grid ---

  int y = 0;
  while (std::getline(in, line))
  {
    // Strip trailing CR so CRLF-terminated files (common on Windows editors)
    // parse the same as LF.
    if (!line.empty() && line.back() == '\r') line.pop_back();

    if (y >= HEIGHT)
    {
      throw std::runtime_error("Too many grid rows in " + path.string() +
                               " (expected " + std::to_string(HEIGHT) + ")");
    }
    // Pad short lines with spaces (Void) but reject over-long lines to catch
    // authoring mistakes.
    if (static_cast<int>(line.size()) > WIDTH)
    {
      throw std::runtime_error(
          "Row " + std::to_string(y) + " in " + path.string() +
          " is too wide: " + std::to_string(line.size()) + " chars (expected " +
          std::to_string(WIDTH) + ")");
    }
    if (static_cast<int>(line.size()) < WIDTH)
    {
      line.append(WIDTH - line.size(), ' ');
    }

    for (int x = 0; x < WIDTH; ++x)
    {
      char c = line[x];
      TileType type = charToRoomTile(c);
      room.tiles[x][y] = Tile(type, Coordinate(x, y));
      if (type == TileType::Door)
      {
        DoorNumber label = c - '0';
        auto [it, inserted] = room.doors.emplace(label, Coordinate{x, y});
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

  if (y != HEIGHT)
  {
    throw std::runtime_error("Not enough grid rows in " + path.string() +
                             ": got " + std::to_string(y) + ", expected " +
                             std::to_string(HEIGHT));
  }

  return room;
}

Coordinate Room::doorAt(DoorNumber number) const
{
  auto it = doors.find(number);
  if (it == doors.end())
  {
    throw std::runtime_error("room " + std::to_string(roomID) + " (" + name +
                             ") has no door " + std::to_string(number));
  }
  return it->second;
}
