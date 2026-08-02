#include "world/map/room.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

// Trim leading/trailing whitespace from a string in place.
std::string trim(std::string s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

// Parse a single grid character into its TileType. Letters A-Z / a-z are
// treated as door labels (returned as Door here; the label itself is captured
// separately by the caller for ordering). Spawn markers (`!`, `$`, `?`) are
// authored on top of walkable floor and therefore resolve to Floor here; the
// spawn coordinate is recorded separately by the caller.
TileType charToRoomTile(char c) {
  if (c == '#') return TileType::Wall;
  if (c == '.') return TileType::Floor;
  if (c == 'o') return TileType::Pillar;
  if (c == ' ') return TileType::Void;
  if (c == '!' || c == '$' || c == '?') return TileType::Floor;
  if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) return TileType::Door;
  std::ostringstream oss;
  oss << "Unrecognized room-file character: '" << c << "' (0x" << std::hex
      << static_cast<int>(static_cast<unsigned char>(c)) << ")";
  throw std::runtime_error(oss.str());
}

// Category a spawn-marker character belongs to. `None` is returned for every
// non-marker character so the caller can branch uniformly.
enum class SpawnKind { None, Enemy, Loot, Item };

SpawnKind charToSpawnKind(char c) {
  switch (c) {
    case '!':
      return SpawnKind::Enemy;
    case '$':
      return SpawnKind::Loot;
    case '?':
      return SpawnKind::Item;
    default:
      return SpawnKind::None;
  }
}

}  // namespace

Room::Room(int id)
    : roomID(id),
      tiles(WIDTH, std::vector<Tile>(HEIGHT)),
      visible(WIDTH, std::vector<bool>(HEIGHT, false)),
      explored(WIDTH, std::vector<bool>(HEIGHT, false)) {}

void Room::clearVisible() {
  for (auto& col : visible) {
    std::fill(col.begin(), col.end(), false);
  }
}

bool Room::isVisible(int x, int y) const {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
  return visible[x][y];
}

bool Room::isExplored(int x, int y) const {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return false;
  return explored[x][y];
}

void Room::reveal(int x, int y) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  visible[x][y] = true;
  // Only mark explored if the tile is not Void.
  if (tiles[x][y].getType() != TileType::Void) explored[x][y] = true;
}

Room Room::loadFromFile(int roomID, const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Could not open room file: " + path.string());
  }

  Room room(roomID);

  // --- Parse header ---
  // Header lines start with '@'. Grid begins at the first non-'@', non-empty
  // line. Blank lines before the grid are permitted so authors can space out
  // the header visually.
  std::string line;
  std::streampos gridStart = in.tellg();
  while (std::getline(in, line)) {
    std::string trimmed = trim(line);
    if (trimmed.empty()) {
      gridStart = in.tellg();
      continue;
    }
    if (trimmed[0] != '@') {
      // Rewind — this line belongs to the grid.
      in.clear();
      in.seekg(gridStart);
      break;
    }

    // Parse "@key: value".
    auto colon = trimmed.find(':');
    if (colon == std::string::npos) {
      throw std::runtime_error("Malformed header (no colon) in " +
                               path.string() + ": " + trimmed);
    }
    std::string key = trim(trimmed.substr(1, colon - 1));
    std::string value = trim(trimmed.substr(colon + 1));

    if (key == "name") {
      room.name = value;
    }
    // Other keys (levels, author, ...) are parsed by the library layer or
    // silently ignored here for forward compatibility.

    gridStart = in.tellg();
  }

  // --- Parse grid ---
  // Collect door positions keyed by label so we can insert them into
  // doorPositions in alphabetical order regardless of scan order.
  std::map<char, Coordinate> labelledDoors;

  int y = 0;
  while (std::getline(in, line)) {
    // Strip trailing CR so CRLF-terminated files (common on Windows editors)
    // parse the same as LF.
    if (!line.empty() && line.back() == '\r') line.pop_back();

    if (y >= HEIGHT) {
      throw std::runtime_error("Too many grid rows in " + path.string() +
                               " (expected " + std::to_string(HEIGHT) + ")");
    }
    // Pad short lines with spaces (Void) but reject over-long lines to catch
    // authoring mistakes.
    if (static_cast<int>(line.size()) > WIDTH) {
      throw std::runtime_error(
          "Row " + std::to_string(y) + " in " + path.string() +
          " is too wide: " + std::to_string(line.size()) + " chars (expected " +
          std::to_string(WIDTH) + ")");
    }
    if (static_cast<int>(line.size()) < WIDTH) {
      line.append(WIDTH - line.size(), ' ');
    }

    for (int x = 0; x < WIDTH; ++x) {
      char c = line[x];
      TileType type = charToRoomTile(c);
      room.tiles[x][y] = Tile(type, Coordinate(x, y));
      if (type == TileType::Door) {
        // Normalize to uppercase for consistent ordering; a/A treated same.
        char label =
            static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        auto [it, inserted] = labelledDoors.emplace(label, Coordinate{x, y});
        if (!inserted) {
          throw std::runtime_error("Duplicate door label '" +
                                   std::string(1, label) + "' in " +
                                   path.string());
        }
      }
      switch (charToSpawnKind(c)) {
        case SpawnKind::Enemy:
          room.enemySpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::Loot:
          room.lootSpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::Item:
          room.itemSpawns.push_back(Coordinate(x, y));
          break;
        case SpawnKind::None:
          break;
      }
    }
    ++y;
  }

  if (y != HEIGHT) {
    throw std::runtime_error("Not enough grid rows in " + path.string() +
                             ": got " + std::to_string(y) + ", expected " +
                             std::to_string(HEIGHT));
  }

  // std::map iterates in key order, so doors land in alphabetical sequence.
  for (const auto& [label, coord] : labelledDoors) {
    (void)label;
    room.doorPositions.push_back(coord);
  }

  return room;
}
