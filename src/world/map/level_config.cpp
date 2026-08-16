#include "world/map/level_config.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "world/map/room.h"

namespace {

nlohmann::json readJson(const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("could not open file: " + path.string());

  nlohmann::json j;
  in >> j;
  return j;
}

LevelMeta parseLevelMeta(const nlohmann::json& j) {
  return LevelMeta{
      j.at("id").get<int>(),
      j.at("name").get<std::string>(),
      j.at("description").get<std::string>(),
      j.at("roomCount").get<int>(),
      j.at("startRoomID").get<int>(),
      j.at("bossRoomID").get<int>(),
  };
}

LevelMeta loadLevelMeta(const std::filesystem::path& path) {
  try {
    return parseLevelMeta(readJson(path));
  } catch (const std::exception& e) {
    throw std::runtime_error("malformed level.json at " + path.string() + ": " +
                              e.what());
  }
}

std::map<int, std::vector<RoomEdge>> parseAdjacency(const nlohmann::json& j) {
  std::map<int, std::vector<RoomEdge>> adjacency;
  for (const auto& roomEntry : j.at("rooms")) {
    int id = roomEntry.at("id").get<int>();
    std::vector<RoomEdge> edges;
    for (const auto& conn : roomEntry.at("connections")) {
      edges.push_back(RoomEdge{
          parseDirection(conn.at("direction").get<std::string>()),
          conn.at("to").get<int>(),
      });
    }
    adjacency[id] = std::move(edges);
  }
  return adjacency;
}

std::map<int, std::vector<RoomEdge>> loadAdjacency(
    const std::filesystem::path& path) {
  try {
    return parseAdjacency(readJson(path));
  } catch (const std::exception& e) {
    throw std::runtime_error("malformed map.json at " + path.string() + ": " +
                              e.what());
  }
}

EnemySpawnConfig parseEnemySpawnConfig(const nlohmann::json& j) {
  return EnemySpawnConfig{
      j.at("type").get<std::string>(),
      j.at("tier").get<int>(),
      j.at("min").get<int>(),
      j.at("max").get<int>(),
      j.at("prob_dist").get<double>(),
  };
}

RoomConfig parseRoomConfig(const nlohmann::json& j) {
  RoomConfig config;
  config.id = j.at("id").get<int>();
  config.name = j.at("name").get<std::string>();
  config.ref = j.at("ref").get<std::string>();

  if (j.contains("enemies")) {
    for (const auto& entry : j.at("enemies")) {
      config.enemies.push_back(parseEnemySpawnConfig(entry));
    }
  }
  return config;
}

RoomConfig loadRoomConfig(const std::filesystem::path& path, int expectedId) {
  try {
    RoomConfig config = parseRoomConfig(readJson(path));
    if (config.id != expectedId) {
      throw std::runtime_error("id " + std::to_string(config.id) +
                                " does not match expected id " +
                                std::to_string(expectedId));
    }
    return config;
  } catch (const std::exception& e) {
    throw std::runtime_error("malformed room config at " + path.string() +
                              ": " + e.what());
  }
}

}  // namespace

Direction parseDirection(const std::string& token) {
  if (token == "N") return Direction::North;
  if (token == "E") return Direction::East;
  if (token == "S") return Direction::South;
  if (token == "W") return Direction::West;
  throw std::runtime_error("invalid direction token: '" + token + "'");
}

Coordinate resolveDoorForDirection(const Room& room, Direction dir) {
  for (const Coordinate& door : room.doorPositions) {
    switch (dir) {
      case Direction::North:
        if (door.y == 0) return door;
        break;
      case Direction::South:
        if (door.y == Room::HEIGHT - 1) return door;
        break;
      case Direction::West:
        if (door.x == 0) return door;
        break;
      case Direction::East:
        if (door.x == Room::WIDTH - 1) return door;
        break;
    }
  }
  throw std::runtime_error("room " + std::to_string(room.roomID) + " (" +
                            room.name + ") has no door on the requested edge");
}

LevelConfig loadLevelConfig(const std::filesystem::path& levelDir) {
  LevelConfig config;
  config.meta = loadLevelMeta(levelDir / "level.json");
  config.adjacency = loadAdjacency(levelDir / "map.json");

  for (const auto& [id, edges] : config.adjacency) {
    std::filesystem::path roomPath =
        levelDir / ("room_" + std::to_string(id) + ".json");
    config.rooms[id] = loadRoomConfig(roomPath, id);
  }

  if (static_cast<int>(config.adjacency.size()) != config.meta.roomCount) {
    throw std::runtime_error(
        "level.json roomCount (" + std::to_string(config.meta.roomCount) +
        ") does not match map.json room count (" +
        std::to_string(config.adjacency.size()) + ") in " + levelDir.string());
  }

  return config;
}
