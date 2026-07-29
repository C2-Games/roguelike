#include "world/room_graph.h"

#include <utility>

#include "core/services.h"
#include "world/room_library.h"

RoomGraph::RoomGraph(int roomCount, GameServices& services)
    : roomCount_(roomCount), services_(services) {
  generate();
}

void RoomGraph::addRoom(Room room) {
  int id = room.roomID;
  rooms_.insert({id, std::move(room)});
}

void RoomGraph::generate() {
  // Load the authored room library from disk. Path is relative to the game's
  // working directory (repo root when launched from there).
  RoomLibrary library;
  library.scan("assets/rooms");

  for (int i = 0; i < roomCount_; i++) {
    addRoom(Room::loadFromFile(i, library.pickRandom(services_.rng)));
    adjacency_.push_back({});
  }

  // Currently wires doors in a chain: room 0 ↔ 1 ↔ 2 ↔ … ↔ (N-1).
  //
  // Room 0 has no incoming connection so doorPositions[0] is free to serve
  // as its exit. Every other room reserves doorPositions[0] for the back-link
  // from the previous room, so it currently uses doorPositions[1] as its
  // forward exit.
  for (int i = 0; i < roomCount_ - 1; i++) {
    const Room& roomA = rooms_.at(i);
    const Room& roomB = rooms_.at(i + 1);

    int exitIdx = (i == 0) ? 0 : 1;

    if (static_cast<int>(roomA.doorPositions.size()) <= exitIdx ||
        roomB.doorPositions.empty())
      continue;

    Coordinate doorA = roomA.doorPositions[exitIdx];
    Coordinate doorB = roomB.doorPositions[0];  // all rooms enter at door[0]

    doorConnections_[{i, doorA}] = {i + 1, doorB};  // A → B
    doorConnections_[{i + 1, doorB}] = {i, doorA};  // B → A

    adjacency_[i].push_back(i + 1);
    adjacency_[i + 1].push_back(i);
  }
}

const DoorConnection* RoomGraph::getDoorConnection(int roomID,
                                                   Coordinate doorPos) const {
  auto it = doorConnections_.find({roomID, doorPos});
  if (it != doorConnections_.end()) return &it->second;
  return nullptr;
}
