#include "preload/room_loader.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

#include "objects/tiles/tile.h"
#include "objects/tiles/tile_type.h"
#include "preload/utils/text.h"
#include "preload/utils/tile_glyph.h"

namespace
{

// double-line box-drawing glyphs authored around a door opening; the single
// stub on one side marks the doorway.
constexpr std::array<char32_t, 10> DOOR_CAP_CODEPOINTS = {
    U'╒', U'╓', U'╕', U'╖', U'╘', U'╙', U'╛', U'╜', U'╥', U'╨'};

// double-line box-drawing straight / corner / junction glyphs, all Wall.
constexpr std::array<char32_t, 11> WALL_CODEPOINTS = {
    U'═', U'║', U'╔', U'╗', U'╚', U'╝', U'╠', U'╣', U'╦', U'╩', U'╬'};

template <std::size_t N>
bool inSet(const std::array<char32_t, N>& set, char32_t c)
{
  return std::find(set.begin(), set.end(), c) != set.end();
}

TileType codepointToRoomTile(char32_t c, const std::filesystem::path& path)
{
  if (c == U'.')
  {
    return TileType::Floor;
  }
  if (c == U'o')
  {
    return TileType::Pillar;
  }
  if (c == U' ')
  {
    return TileType::Void;
  }
  if (c == U'E' || c == U'L')
  {
    return TileType::Floor;
  }
  if (c >= U'0' && c <= U'9')
  {
    return TileType::Door;
  }
  if (inSet(DOOR_CAP_CODEPOINTS, c))
  {
    return TileType::DoorCap;
  }
  if (inSet(WALL_CODEPOINTS, c))
  {
    return TileType::Wall;
  }
  std::ostringstream oss;
  oss << "Unrecognized room-file character: U+" << std::hex << std::uppercase
      << std::setw(4) << std::setfill('0') << static_cast<std::uint32_t>(c)
      << " in " << path.string();
  throw std::runtime_error(oss.str());
}

enum class SpawnKind : std::uint8_t
{
  None,
  Enemy,
  LootOrItem
};

SpawnKind codepointToSpawnKind(char32_t c)
{
  switch (c)
  {
    case U'E':
      return SpawnKind::Enemy;
    case U'L':
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
    std::string trimmed = preload::trim(line);
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
    std::string key = preload::trim(trimmed.substr(1, colon - 1));
    std::string value = preload::trim(trimmed.substr(colon + 1));

    if (key == "name")
    {
      room.setName(value);
    }
    // other keys (levels, author, ...) are parsed by the library layer or
    // silently ignored here for forward compatibility.

    gridStart = in.tellg();
  }
}

// place one decoded grid cell: its tile (keeping the authored glyph for the
// visual-only wall/cap variants), plus any door label or spawn point it marks.
void applyGridCell(Room& room, char32_t cp, Coordinate at,
                   const std::filesystem::path& path,
                   std::vector<Coordinate>& enemySpawns,
                   std::vector<Coordinate>& lootSpawns,
                   std::vector<Coordinate>& itemSpawns)
{
  const TileType type = codepointToRoomTile(cp, path);
  Tile tile(type, at);
  // wall and cap cells keep the authored box-drawing glyph; every other cell
  // takes its type's default, since its authored char is a semantic marker
  // (a door digit, a spawn letter) rather than the glyph to render.
  if (type == TileType::Wall || type == TileType::DoorCap)
  {
    tile.setSymbol(static_cast<wchar_t>(cp));
  }
  else
  {
    tile.setSymbol(preload::defaultGlyph(type));
  }
  room.setTile(at, tile);

  if (type == TileType::Door)
  {
    const DoorNumber label = static_cast<DoorNumber>(cp - U'0');
    if (room.getDoors().contains(label))
    {
      throw std::runtime_error("Duplicate door label '" +
                               std::to_string(label) + "' in " + path.string());
    }
    room.addDoor(label, at);
  }

  switch (codepointToSpawnKind(cp))
  {
    case SpawnKind::Enemy:
      enemySpawns.push_back(at);
      break;
    case SpawnKind::LootOrItem:
      lootSpawns.push_back(at);
      itemSpawns.push_back(at);
      break;
    case SpawnKind::None:
      break;
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

    // the grid is authored in multi-byte box-drawing glyphs, so work in
    // codepoints from here on.
    std::vector<char32_t> cps = preload::decodeUtf8(line, path, y);

    // pad short lines with spaces (Void) but reject over-long lines to catch
    // authoring mistakes.
    if (static_cast<int>(cps.size()) > Room::WIDTH)
    {
      throw std::runtime_error(
          "Row " + std::to_string(y) + " in " + path.string() +
          " is too wide: " + std::to_string(cps.size()) + " chars (expected " +
          std::to_string(Room::WIDTH) + ")");
    }
    if (static_cast<int>(cps.size()) < Room::WIDTH)
    {
      cps.resize(static_cast<std::size_t>(Room::WIDTH), U' ');
    }

    for (int x = 0; x < Room::WIDTH; ++x)
    {
      applyGridCell(room, cps[x], Coordinate(x, y), path, enemySpawns,
                    lootSpawns, itemSpawns);
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

Coordinate inwardOfDoor(const Room& room, Coordinate doorPos)
{
  // a door sits in a wall run; of its four orthogonal neighbours exactly one
  // is walkable interior floor (the others are wall, cap, or outside the
  // room), so step onto that one. edge position isn't reliable — the
  // re-authored rooms inset their walls.
  const std::array<Coordinate, 4> neighbours = {
      Coordinate{doorPos.x, doorPos.y + 1},
      Coordinate{doorPos.x, doorPos.y - 1},
      Coordinate{doorPos.x + 1, doorPos.y},
      Coordinate{doorPos.x - 1, doorPos.y}};
  const auto* const walkable = std::find_if(
      neighbours.begin(), neighbours.end(),
      [&](const Coordinate& neighbour) { return room.isWalkable(neighbour); });
  return walkable != neighbours.end() ? *walkable : doorPos;
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
    Coordinate entry = inwardOfDoor(room, doorPos);
    Tile entryTile(TileType::EntryWay, entry);
    entryTile.setSymbol(preload::defaultGlyph(TileType::EntryWay));
    room.setTile(entry, entryTile);
  }

  return ParsedRoom{std::move(room), std::move(enemySpawns),
                    std::move(lootSpawns), std::move(itemSpawns)};
}

}  // namespace room_loader
