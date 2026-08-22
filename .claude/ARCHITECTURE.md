# Roguelike Architecture

## Overview

Single-executable terminal roguelike (target `roguelike`), plain OOP (no ECS), ncurses rendering.

`src/main.cpp` constructs a `UIManager` (`include/io/ui_manager.h`, `src/io/ui_manager.cpp` — sets up ncurses) and then a `Game` (`include/core/game.h`, `src/core/game.cpp`) that takes that `UIManager&`, which owns essentially all game state and *is* the loop (a prior refactor removed a separate `Level`/stage abstraction — `Game::run()` now does `handleInput() → update() → render()` each frame, paced to `fps_`). `Game` owns: `GameServices` (holds two `std::mt19937` engines — `rng` for spawn/level generation, `movementRng` for enemy movement — injected by reference everywhere instead of using globals/statics, both seeded from a fixed constant for reproducible runs), `Player`, `Level` (the *room graph* data structure, not a stage/scene), `GoalMapCache`, and a `vector<unique_ptr<Projectile>>`, plus a non-owning `UIManager&` reference it routes all I/O through. Enemies are not a `Game` member — they live per-`Room` (see below).

## Input

`include/io/input/`, `src/io/input/`: `input::pollInput()` (`handle_input.h`/`.cpp`) is the sole place that calls ncurses `getch()`/reads `KEY_*` constants; it returns a `GameCommand` (`game_commands.h`: `None`/`MoveUp`/`MoveDown`/`MoveLeft`/`MoveRight`/`Attack`/`Quit`/`Resize`), which `Game::handleInput()` switches on. All movement/walkability/door-transition/projectile-spawn decision logic still lives in `Game`, unchanged — only how it learns what was pressed moved out. `UIManager::pollInput()` intercepts `GameCommand::Resize` itself (resizing the render layers) and reports it back to `Game` as `GameCommand::None`, so `Game` never observes a resize — `Game::handleInput()`'s switch keeps an unreachable `case GameCommand::Resize: break;` purely so the switch stays exhaustive over the enum.

## World/map

`include/world/map/`, `src/world/map/`:
- `Level` — room graph: `rooms_` map and `doorConnections_` (type `LevelMap`) keyed by `(roomID, doorPos)`. Built by `level_loader::loadLevel(levelDir, services, catalog)` (`include/preload/level_loader.h`, `src/preload/level_loader.cpp`), which parses the level config via `loadLevelConfig`, loads each room via `room_loader::loadRoom`, wires door adjacency, seals unlinked doors, then constructs `Level` through a new data constructor `Level(LevelMeta meta, std::map<int, Room> rooms, LevelMap doorConnections, GameServices& services)` — `Level` itself does no file I/O anymore (its old `Level(levelDir, services, catalog)` constructor and private `buildFromConfig`/`sealUnlinkedDoors` methods are gone). `map.json` holds a `rooms` id list plus a flat `edges` array; each edge names both endpoints by room and **door number** (`{"from": {"room": 1, "door": 2}, "to": {"room": 3, "door": 4}}`), resolved against the room's authored door labels via `Room::doorAt`. Each edge is stated once and wired in both directions, so the graph cannot be asymmetric by construction. Doors a level never names are sealed back to `Wall` by `sealUnlinkedDoors()` — a room template may carry more doors than any one level uses.
- `Room` — a plain data structure with no parsing responsibility: fixed grid (`WIDTH=175`, `HEIGHT=50`) of `Tile`; it no longer has a `loadFromFile` static method. Rooms are loaded via the free function `room_loader::loadRoom` (`include/preload/room_loader.h`, `src/preload/room_loader.cpp`) from hand-authored files in `assets/rooms/*.txt`: a `@key: value` header followed by an ASCII grid. Legend: `#`=Wall, `.`=Floor, `o`=Pillar, space=Void, `E`=enemy spawn, `L`=loot/item spawn, digits=numbered door tiles (collected into `doors`, a `map<DoorNumber, Coordinate>` keyed by the authored digit — labels need not be dense, and `Room::doorAt` throws on an unknown one). Also owns a `RoomEnemyState` member (`enemyState`) holding that room's live enemies, exposed via `enemies()`/`ensureEnemiesSpawned()`.

## Entities

`include/entities/`, `src/entities/`: `Entity` is the abstract base (`Coordinate` position, `EntitySymbol` symbol, health, speed, `FOV`) for `Player` and `Enemy` — both subclasses build their `FOV` (via `ellipseFOV`) and pass it up through `Entity`'s constructor rather than storing their own. `EntitySymbol` (`using EntitySymbol = std::vector<std::vector<char>>`, defined in `entity.h`) is a multi-cell glyph grid; a cell holding `'\0'` renders as transparent (the floor tile beneath shows through) instead of a literal blank glyph. `Entity` also carries a shared `EntityAction` enum (`Attack`/`Move`/`Idle`/`Damaged`/`Ability`/`TransRoom`) via a protected `state_` member and a public `getState()` getter, used directly by both `Player` and `Enemy` with no per-subclass override; it's currently set by `moveHook` (`Move`, on an actual throttled move committing) and by both `Player::takeDamage` and `Enemy::takeDamage` (`Damaged`), plus `Enemy::moveTowardPlayer` (`Attack`, on a melee attack attempt, overriding whatever `Move` moveHook set moments earlier in the same call) — `Attack` from the player's side, `TransRoom`, and `Ability` aren't wired up yet, pending a follow-up issue that restructures `Game::update()`'s per-frame ordering. `Enemy::moveTowardPlayer` uses the goal-map/pathfinding system; target selection is driven by `Enemy`'s own `AIState` (`Sentry`/`Chase`/`Search`, stored in a private `aiState_` member distinct from the inherited `state_`), recomputed each frame by `transitionState()` from FoV and "chase memory" (`lastKnownPlayerPos_`/`chaseTurnsRemaining_`) before movement runs, with empty `onEnterX()` seams for future per-state behavior; `Sentry` is the patrol/wander default, serving both as the state before the enemy has ever spotted the player and as where it returns after giving up a search. The inherited `fov_` doubles as its detection/chase-trigger radius, not a separate attack range. `EnemyCatalog` (`include/preload/enemy_catalog.h`, `src/preload/enemy_catalog.cpp`) parses every file in `assets/enemies/` into a `(name, tier) → stats` lookup, including each enemy's nested `symbol` JSON array into an `EntitySymbol`. Enemy spawning is `enemy_factory::rollForRoom`, which shuffles the room's `E` spawn points, then for each entry in the room's authored spawn tablerolls uniform_int over the entry's authored range for how many to place, resolving stats through `EnemyCatalog::find`. `RoomEnemyState` (a member of each `Room`) holds that room's live enemies (HP/position persist across room transitions) and lazily rolls spawns on first visit via `ensureSpawned(room, services)`.

## Preload

`preload/` (`include/preload/`, `src/preload/`) is a module sibling to `core/`, `entities/`, `world/`, `io/` that owns all level/room/enemy config-file loading — no other part of the game-logic layer does file I/O or JSON/text parsing.

- `EnemyCatalog` (`include/preload/enemy_catalog.h`, `src/preload/enemy_catalog.cpp`) — parses `assets/enemies/*.json`, unchanged behavior from before the move.
- `room_loader::loadRoom` (`include/preload/room_loader.h`, `src/preload/room_loader.cpp`) — parses one `assets/rooms/*.txt` file into a `Room`.
- `level_loader.h`/`.cpp` (`include/preload/level_loader.h`, `src/preload/level_loader.cpp`) — holds the config value types (`LevelMeta`, `RoomConfig`, `RoomAdjacency`, `EnemySpawnConfig`, `LevelConfig`, `DoorNumber`) plus `loadLevelConfig` for raw JSON parsing, and the higher-level `level_loader::loadLevel` that orchestrates the whole level build: parse config, load every room, spawn enemies, wire adjacency, seal unlinked doors, return a fully-built `Level`.

`Game` constructs its `level_` member via `level_loader::loadLevel("assets/levels/level_1", services_, enemyCatalog_)` in its constructor's initializer list.

## Pathfinding

`include/world/systems/`, `src/world/systems/`: `pathfinding::computeGoalMap` builds a Dijkstra/BFS distance-to-goal grid per room. `GoalMapCache` caches these keyed by `(roomID, goal)`, capped at 32 entries (clears entirely, not LRU, once full). `visibility::update` recomputes FoV/fog-of-war each frame; `FOV` (`include/entities/fov.h`) is the abstract shape interface, with `ellipseFOV`/`EllipseFOV` (`include/entities/ellipse_fov.h`) building the offset-set ellipse shape, aspect-corrected for terminal cells.

## Rendering

`include/io/output/`, `src/io/output/`: `RenderStack` is an abstract base wrapping one ncurses `WINDOW*`, providing shared window/enable/resize/`doUpdate()` (per-frame state hook, default no-op — currently unused by any layer, but kept as general infrastructure) infrastructure — layers hold no live references into `Game`'s object graph, only geometry from their constructor. There is no shared render-call contract: each of the four layers declares its own `doRender(const XxxLayerPacket&)` taking only the data it needs (`MapLayer::doRender(const MapLayerPacket&)`, `EntityLayer::doRender(const EntityLayerPacket&)`, `HUDLayer::doRender(const HUDLayerPacket&)`, `DebugLayer::doRender(const DebugLayerPacket&)`). `UIManager` owns the four layers directly as typed `unique_ptr` members and composes them itself each frame — no separate `Renderer` class exists — routing each named `RenderState` field (`RenderState::map`/`entity`/`hud`/`debug`) to its matching layer. Layers, constructed in `UIManager`'s constructor: `MapLayer` → `EntityLayer` (draws only what's in the player's FoV) → `HUDLayer` → `DebugLayer` (compiled in only when `NDEBUG` is not defined, i.e. debug builds). `Game::render()` builds a fresh `RenderState` snapshot every frame via `render_state_builder::build()` (`core/render_state_builder.h`/`.cpp`) and pushes it through `uiManager_.render()`. Render-effect state such as the player hit-flash now lives on `Game` (`playerHitFlashFramesRemaining_`), not on any layer — the builder resolves it into the snapshot's `RenderState::entity.player` tint fields each frame, so no raw pointer into a layer exists anymore. `core/colors.h` keeps the plain `ColorPair` enum as shared vocabulary; `io/output/colors.h` holds the ncurses-specific `colorAttr()`/`initColors()`.

## Conventions

Trailing underscore for private members; `#ifndef` include guards (no `#pragma once`); PascalCase classes, snake_case free-function namespaces (`enemy_factory::`, `visibility::`); `getX()`/`isX()` accessors; `unique_ptr` for polymorphic ownership, plain references for non-owning per-frame "context" structs (`FrameState`) instead of globals; Doxygen-style `/** @brief */` header comments; heavy forward-declaration use to keep header coupling low.

## Logging

`Logger::get()` singleton writes to `game.log`/`error.log` at the repo root via `LOG(msg)`/`LOG_ERR(msg)` macros.

## Known gaps

`assets/enemies/` holds a single flat `goblin.json` — the `{class}/` nesting the level format anticipates does not exist yet, and neither do `assets/drops/` or `assets/items/`.

## io/

Both input & output modules are completely stateless and should have no
knowledge of game-specific components (`Player`, `Level`, `Room`, `Enemy`,
`Projectile`). `Game` owns all real game state; each frame it hands `io/`
a plain-data snapshot to render and reads back a plain-data command — `io/`
never holds a reference into `Game`'s object graph across frames.

### ui_manager.cpp/.h

The middleware `Game` talks to for all I/O. Owns the four render layers
directly (typed `unique_ptr` members) and ncurses' lifecycle —
`initscr`/`initColors`/`cbreak`/`noecho`/`keypad`/`curs_set` on
construction, `endwin()` on destruction. `main.cpp` only constructs a
`UIManager`, then a `Game`, then calls `game.run()`.

- `GameCommand pollInput()` — delegates to `input::pollInput()`. Intercepts
  `GameCommand::Resize` itself (calls `getmaxyx` then `onResize` on each
  owned layer) rather than surfacing it to `Game`, which has no reason to
  know terminal dimensions — it always returns `GameCommand::None` for a
  resize poll.
- `void render(const RenderState& state)` — routes each named field
  (`state.map`, `state.entity`, `state.hud`, `state.debug`) to its matching
  owned layer's own `doRender(const XxxLayerPacket&)`, then composes.
- `void showStartScreen()` / `void showGameOver()` — the "press SPACE to
  begin" intro and "Game Over!" message, currently raw `printw`/`getch`/
  `std::cout` living in `Game::run()`. Confining them here keeps every
  ncurses call inside `io/`.

### /input/

#### game_commands.h

```cpp
enum class GameCommand
{
  None,
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  Attack,
  Quit,
  Resize,  // consumed by UIManager; Game never observes this value
};
```

#### handle_input.cpp/.h

`GameCommand pollInput()` — the only place `getch()`/`KEY_*` constants are
read. Pure ncurses-key → `GameCommand` mapping; no movement, collision,
door, or projectile-spawn logic (that stays in `Game`, driven off the
returned command, exactly as it runs today inside `Game::handleInput()`).
An unmapped key resolves to `GameCommand::None` (the current
`mvprintw(..., "Invalid key")` debug branch is dropped — it reached past
the I/O boundary to print game-facing text).

### /output/

* Completely stateless: layers hold no live references to `Player`/`Level`/
  `Room` — only geometry (from their constructor) and whatever
  per-layer packet they're handed this frame.
* On every frame, `UIManager` owns the four typed layer instances directly
  and composes them itself: `render(const RenderState& state)` explicitly
  routes each packet (`state.map`, `state.entity`, `state.hud`,
  `state.debug`) to its corresponding layer's own
  `doRender(const XxxLayerPacket&)` — there is no `Renderer` class and no
  shared `doRender` signature.
* `render_stack.h`, `window_position.h`, and `layers/` move here from the
  current top-level `render/` tree; `RenderStack` keeps only shared
  window/enable/resize/`doUpdate()` infrastructure — each layer declares
  its own `doRender(const XxxLayerPacket&)` taking only the data it needs.
* `colors.h` splits: the plain `ColorPair` enum stays as shared vocabulary
  (referenced by `Weapon` and other game-side code); `colorAttr()` and
  `initColors()` — the actual ncurses calls — move here.

#### render_state.h

The plain-data snapshot type. No `Player`/`Level`/`Room`/`Enemy` types
appear in it — only value types already used as shared vocabulary
(`Coordinate`, `EntitySymbol`, `ColorPair`).

```cpp
enum class TileVisibility { Visible, Explored, Unseen };

struct TileView
{
  char symbol;
  TileVisibility visibility;
};

struct EntityView
{
  Coordinate position;
  EntitySymbol symbol;
  bool tinted;         // whether to apply tintColor over the symbol this frame
  ColorPair tintColor;  // meaningful only when tinted == true
};

struct ProjectileView
{
  Coordinate position;
  ColorPair color;
};

struct WeaponView
{
  std::string name;
  int damage, speed, range;
  ColorPair color;
};

struct MapLayerPacket
{
  std::array<std::array<TileView, Room::HEIGHT>, Room::WIDTH> tiles;
};

struct EntityLayerPacket
{
  EntityView player;
  std::vector<EntityView> enemies;          // already alive- and visibility-filtered
  std::vector<ProjectileView> projectiles;  // already active- and visibility-filtered
};

struct HUDLayerPacket
{
  int playerHealth, playerMaxHealth;
  int roomIndex, roomCount;
  WeaponView weapon;
};

struct DebugLayerPacket
{
  Coordinate playerPosition;
  double fps;
};

struct RenderState
{
  MapLayerPacket map;
  EntityLayerPacket entity;
  HUDLayerPacket hud;
  DebugLayerPacket debug;
};
```

Assembled each frame by a new `render_state_builder::build(...)` free
function living in `core/` (it depends on `Player`/`Level`/`Projectile`, so
it belongs on the game-specific side of the boundary, not inside `io/`).
`Game::render()` becomes `uiManager_.render(render_state_builder::build(
player_, level_, projectiles_, currentFps_, playerHitFlashFramesRemaining_
> 0))`.

Per-tile visibility (today's `Room::isVisible`/`isExplored`) is baked into
`TileView` once per tile for `MapLayerPacket`. Enemy and projectile
visibility is resolved upstream of that, in `render_state_builder::build()`
itself: each is checked against `room.isVisible(...)` on its own origin
position before it's pushed onto `EntityLayerPacket::enemies`/`projectiles`
at all, so `EntityLayer` never holds a `const Room*` and does no filtering
of its own — it draws exactly the (already alive-, active-, and
visibility-filtered) list it's handed.

**No counters on the render side.** `io/output/` holds zero frame-to-frame
state — not even render-effect timers. The player hit-flash countdown
(today's `EntityLayer::hitFlashFramesRemaining_`, ticked in `doUpdate()`)
moves to a `Game`-owned counter (`playerHitFlashFramesRemaining_`), reset
to `HIT_FLASH_FRAMES` when an enemy's attack lands and decremented once per
frame in `Game::update()`. Each frame, `render_state_builder::build()`
resolves that counter down to the already-decided `EntityView::tinted` /
`tintColor` pair on `RenderState::entity.player` — `io/` only ever renders
what it's told this instant; it never owns or advances a timer. `EntityLayer`
loses its `doUpdate()` override, `triggerPlayerHitFlash()`, and the
`hitFlashFramesRemaining_` member entirely — it becomes a pure
`doRender(const EntityLayerPacket&)` with no state of its own.