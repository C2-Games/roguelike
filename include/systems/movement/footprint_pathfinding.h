#ifndef FOOTPRINT_PATHFINDING_H
#define FOOTPRINT_PATHFINDING_H

#include "objects/coordinate.h"
#include "objects/entities/entity_symbol.h"
#include "systems/movement/pathfinding.h"

struct Room;

// per-orientation distance-to-goal grids for a footprint-aware BFS.
struct FootprintGoalMap
{
  GoalMap horizontal;
  GoalMap vertical;
};

/**
 * @brief Build a configuration-space (anchor, orientation) BFS goal map for
 * a multi-tile footprint.
 *
 * @param room The room whose tile grid is used for wall lookups.
 * @param goal The tile a footprint should end up orthogonally adjacent to
 * (e.g. the player's position) -- not a tile the footprint occupies.
 * @param baseSymbol The entity's always-Horizontal-authored symbol grid.
 * @return Distance grids, one per orientation, each sized Room::WIDTH x
 * Room::HEIGHT and indexed by anchor coordinate.
 */
FootprintGoalMap computeFootprintGoalMap(const Room& room, Coordinate goal,
                                         const EntitySymbol& baseSymbol);

#endif
