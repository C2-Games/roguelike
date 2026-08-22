# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Workflow (enforced, not advisory)

1. **No change without a GitHub issue.** Run `/start-issue <number>` before editing anything under
   `src/`, `include/`, `assets/`, or the build files. A `PreToolUse` hook denies those edits until
   an issue is recorded for the current branch, and always denies edits made on `main`. One branch
   may carry several issues — record them all. If a change is requested directly, without
   `/start-issue` having been run, dispatch the `issue-drafter` agent the same way `/new-issue`
   does — it matches a template, asks follow-ups, and waits for explicit approval — before
   creating the issue and branch and proceeding. For parallel issues, `/start-issue` can isolate work in a git
   worktree (`.claude/worktrees/<branch>`) instead of switching the current checkout — see the
   Lifecycle diagram in WORKFLOW.md.
2. **Format and static analysis run once per change, at `/check`.** Nothing is checked while you
   write — there is no write-time hook. `/check` applies `clang-format -i` over `src/`/`include/`,
   then runs `scripts/ci-local.sh` (cppcheck, clang-tidy, build) in the background, since the first
   run can take minutes, then — when `src/` or `include/` actually changed on the branch —
   dispatches the `reviewer` agent for a read-only structure/efficiency/isolation pass (grounded in
   the originating issue and the `cpp-style` skill, run asynchronously), then stamps
   `.claude/.last-check` with a hash of `src/`/`include/` that `/pr` verifies before drafting a
   body. It is the required final step of every plan. A single-file, purely documentation/comment or
   one-line fix may skip formal plan mode; anything touching `src/`/`include/` behavior, spanning
   multiple files, or otherwise non-trivial still requires a plan before editing.
3. **Never `git commit` or `git push`.** Both are denied in `.claude/settings.json`; publishing is
   the user's call. `/pr` prepares the title and body and hands the commands back (after
   confirming `/check` is current, and keeping bodies short — no decision narration).
4. **This file is maintained with the code, not after it.** Every plan states whether the change
   earns an update here — yes for a new subsystem, a file-layout change, ownership moving between
   types, a changed convention, or a placeholder becoming real; no for renames, small refactors and
   bugfixes. As a backstop, a `Stop` hook (`.claude/hooks/doc_drift.py`) lists any workflow-defining
   file that changed while this one did not, once per distinct set of changes.

Commands: `/issues`, `/new-issue`, `/start-issue`, `/check`, `/build`, `/run`, `/pr`. The `cpp-style`
skill (`.claude/skills/cpp-style/SKILL.md`) carries the conventions no linter can check — naming,
comment voice, and where a Doxygen docstring is allowed to go — and applies to all C++ work here.
Three project agents back issue filing, implementation, and review: `.claude/agents/issue-drafter.md`
drafts and files GitHub issues (dispatched by `/new-issue` and by any direct change request with no
issue on record), asking follow-ups on implementation approach, whether to split into multiple
issues, parent/sub-issue linkage, and milestone before creating anything;
`.claude/agents/implementer.md` executes one isolated plan task at a time (dispatched in parallel
when tasks are independent); and `.claude/agents/reviewer.md` gives `/check` a read-only
structure/efficiency/isolation/style pass over `src/`/`include/` changes, grounded in the
originating issue.

@.claude/WORKFLOW.md

## Build & run

Build scripts are bash (`scripts/*.sh`) — on Windows use Git Bash or WSL.

- Release build: `./scripts/build-release.sh` → binary at `.build/release/roguelike`
- Debug build (enables FPS/coordinate debug overlay): `./scripts/build-debug.sh` → binary at `.build/debug/roguelike`
- Manual equivalent: `cmake -B .build/release -S . && cmake --build .build/release`
- Run from a directory containing `assets/` (repo root, or the build output needs `assets/` alongside it): `./.build/release/roguelike`

Dependencies: wide-char ncurses (`libncurses-dev` on Debian/Ubuntu/WSL, `brew install ncurses` on macOS) and `nlohmann_json` (fetched automatically via CMake `FetchContent` if not found on the system). CMake 3.16+, C++20.

There is no test suite in this repo (no `tests/` directory, no CTest/GoogleTest/Catch2 wiring). CI correctness gates are formatting, static analysis, and a successful build only.

## Lint / format / local CI

- `scripts/ci-local.sh` replicates the CI checks locally (format check, cppcheck, clang-tidy, then build) using a `build/` directory with `compile_commands.json` — run this before pushing.
- Format: `.clang-format` (Google style, with Allman braces — every brace opens on its own line; empty bodies stay `{}`). Check with `clang-format --dry-run --Werror -style=file` over `src/`+`include/` `.cpp`/`.h`/`.hpp` files.
- Static analysis: `.clang-tidy` enables `bugprone-*`, `performance-*`, `readability-*` (minus `magic-numbers` and `identifier-length`), and requires clang-tidy 19+ (invoked as `clang-tidy-19`) for `HeaderFilterRegex`/`ExcludeHeaderFilterRegex` to surface warnings from this project's own `include/` headers while excluding the fetched nlohmann/json.hpp and system ncurses headers. Also `cppcheck --enable=all` and CodeQL run in CI.
- Blame hygiene: `.git-blame-ignore-revs` lists formatting-only commits (the Allman/LF reformat).
  GitHub's web blame reads it automatically; **locally each clone needs it enabled once** with
  `git config blame.ignoreRevsFile .git-blame-ignore-revs`. Add future whitespace-only commits to it.
- CI (`.github/workflows/ci.yml`) builds Linux + macOS only — no Windows build/target.
- Releases (`.github/workflows/release.yml`) trigger when the `VERSION` file changes on `main`, tagging `v<VERSION>` and publishing Linux/macOS binaries.

## Architecture

Single-executable terminal roguelike (target `roguelike`), plain OOP (no ECS), ncurses rendering.

**Entry point & game loop**: `src/main.cpp` constructs a `UIManager` (`include/io/ui_manager.h`, `src/io/ui_manager.cpp` — sets up ncurses) and then a `Game` (`include/core/game.h`, `src/core/game.cpp`) that takes that `UIManager&`, which owns essentially all game state and *is* the loop (a prior refactor removed a separate `Level`/stage abstraction — `Game::run()` now does `handleInput() → update() → render()` each frame, paced to `fps_`). `Game` owns: `GameServices` (holds two `std::mt19937` engines — `rng` for spawn/level generation, `movementRng` for enemy movement — injected by reference everywhere instead of using globals/statics, both seeded from a fixed constant for reproducible runs), `Player`, `Level` (the *room graph* data structure, not a stage/scene), `GoalMapCache`, and a `vector<unique_ptr<Projectile>>`, plus a non-owning `UIManager&` reference it routes all I/O through. Enemies are not a `Game` member — they live per-`Room` (see below).

**Input** (`include/io/input/`, `src/io/input/`): `input::pollInput()` (`handle_input.h`/`.cpp`) is the sole place that calls ncurses `getch()`/reads `KEY_*` constants; it returns a `GameCommand` (`game_commands.h`: `None`/`MoveUp`/`MoveDown`/`MoveLeft`/`MoveRight`/`Attack`/`Quit`/`Resize`), which `Game::handleInput()` switches on. All movement/walkability/door-transition/projectile-spawn decision logic still lives in `Game`, unchanged — only how it learns what was pressed moved out. `UIManager::pollInput()` intercepts `GameCommand::Resize` itself (resizing the render layers) and reports it back to `Game` as `GameCommand::None`, so `Game` never observes a resize — `Game::handleInput()`'s switch keeps an unreachable `case GameCommand::Resize: break;` purely so the switch stays exhaustive over the enum.

**World/map** (`include/world/map/`, `src/world/map/`):
- `Level` — room graph: `rooms_` map and `doorConnections_` (type `LevelMap`) keyed by `(roomID, doorPos)`. Built by `Level::Level(levelDir, services, catalog)` from a level config directory (`assets/levels/<name>/level.json` + `map.json` + `room_<id>.json`, parsed via `world/map/level_config.h`'s `loadLevelConfig`). `map.json` holds a `rooms` id list plus a flat `edges` array; each edge names both endpoints by room and **door number** (`{"from": {"room": 1, "door": 2}, "to": {"room": 3, "door": 4}}`), resolved against the room's authored door labels via `Room::doorAt`. Each edge is stated once and wired in both directions, so the graph cannot be asymmetric by construction. Doors a level never names are sealed back to `Wall` by `sealUnlinkedDoors()` — a room template may carry more doors than any one level uses.
- `Room` — fixed grid (`WIDTH=175`, `HEIGHT=50`) of `Tile`, loaded from hand-authored files in `assets/rooms/*.txt` via `Room::loadFromFile`: a `@key: value` header followed by an ASCII grid. Legend: `#`=Wall, `.`=Floor, `o`=Pillar, space=Void, `E`=enemy spawn, `L`=loot/item spawn, digits=numbered door tiles (collected into `doors`, a `map<DoorNumber, Coordinate>` keyed by the authored digit — labels need not be dense, and `Room::doorAt` throws on an unknown one). Also owns a `RoomEnemyState` member (`enemyState`) holding that room's live enemies, exposed via `enemies()`/`ensureEnemiesSpawned()`.

**Entities** (`include/entities/`, `src/entities/`): `Entity` is the abstract base (`Coordinate` position, `EntitySymbol` symbol, health, speed, `FOV`) for `Player` and `Enemy` — both subclasses build their `FOV` (via `ellipseFOV`) and pass it up through `Entity`'s constructor rather than storing their own. `EntitySymbol` (`using EntitySymbol = std::vector<std::vector<char>>`, defined in `entity.h`) is a multi-cell glyph grid; a cell holding `'\0'` renders as transparent (the floor tile beneath shows through) instead of a literal blank glyph. `Enemy::moveTowardPlayer` uses the goal-map/pathfinding system; target selection is driven by an explicit `AIState` (`Idle`/`Chasing`/`Searching`), recomputed each frame by `transitionState()` from FoV and "chase memory" (`lastKnownPlayerPos_`/`chaseTurnsRemaining_`) before movement runs, with empty `onEnterX()` seams for future per-state behavior. The inherited `fov_` doubles as its detection/chase-trigger radius, not a separate attack range. `EnemyCatalog` parses every file in `assets/enemies/` into a `(name, tier) → stats` lookup, including each enemy's nested `symbol` JSON array into an `EntitySymbol`. Enemy spawning is `enemy_factory::rollForRoom`, which shuffles the room's `E` spawn points, then for each entry in the room's authored spawn tablerolls uniform_int over the entry's authored range for how many to place, resolving stats through `EnemyCatalog::find`. `RoomEnemyState` (a member of each `Room`) holds that room's live enemies (HP/position persist across room transitions) and lazily rolls spawns on first visit via `ensureSpawned(room, services)`.

**Known gaps**: `assets/enemies/` holds a single flat `goblin.json` — the `{class}/` nesting the level format anticipates does not exist yet, and neither do `assets/drops/` or `assets/items/`.

**Pathfinding** (`include/world/systems/`, `src/world/systems/`): `pathfinding::computeGoalMap` builds a Dijkstra/BFS distance-to-goal grid per room. `GoalMapCache` caches these keyed by `(roomID, goal)`, capped at 32 entries (clears entirely, not LRU, once full). `visibility::update` recomputes FoV/fog-of-war each frame; `FOV` (`include/entities/fov.h`) is the abstract shape interface, with `ellipseFOV`/`EllipseFOV` (`include/entities/ellipse_fov.h`) building the offset-set ellipse shape, aspect-corrected for terminal cells.

**Rendering** (`include/io/output/`, `src/io/output/`): `RenderStack` is an abstract base wrapping one ncurses `WINDOW*`, providing shared window/enable/resize/`doUpdate()` (per-frame state hook, default no-op — currently unused by any layer, but kept as general infrastructure) infrastructure — layers hold no live references into `Game`'s object graph, only geometry from their constructor. There is no shared render-call contract: each of the four layers declares its own `doRender(const XxxLayerPacket&)` taking only the data it needs (`MapLayer::doRender(const MapLayerPacket&)`, `EntityLayer::doRender(const EntityLayerPacket&)`, `HUDLayer::doRender(const HUDLayerPacket&)`, `DebugLayer::doRender(const DebugLayerPacket&)`). `UIManager` owns the four layers directly as typed `unique_ptr` members and composes them itself each frame — no separate `Renderer` class exists — routing each named `RenderState` field (`RenderState::map`/`entity`/`hud`/`debug`) to its matching layer. Layers, constructed in `UIManager`'s constructor: `MapLayer` → `EntityLayer` (draws only what's in the player's FoV) → `HUDLayer` → `DebugLayer` (compiled in only when `NDEBUG` is not defined, i.e. debug builds). `Game::render()` builds a fresh `RenderState` snapshot every frame via `render_state_builder::build()` (`core/render_state_builder.h`/`.cpp`) and pushes it through `uiManager_.render()`. Render-effect state such as the player hit-flash now lives on `Game` (`playerHitFlashFramesRemaining_`), not on any layer — the builder resolves it into the snapshot's `RenderState::entity.player` tint fields each frame, so no raw pointer into a layer exists anymore. `core/colors.h` keeps the plain `ColorPair` enum as shared vocabulary; `io/output/colors.h` holds the ncurses-specific `colorAttr()`/`initColors()`.

**Conventions**: trailing underscore for private members; `#ifndef` include guards (no `#pragma once`); PascalCase classes, snake_case free-function namespaces (`enemy_factory::`, `visibility::`); `getX()`/`isX()` accessors; `unique_ptr` for polymorphic ownership, plain references for non-owning per-frame "context" structs (`FrameState`) instead of globals; Doxygen-style `/** @brief */` header comments; heavy forward-declaration use to keep header coupling low.

**Logging**: `Logger::get()` singleton writes to `game.log`/`error.log` at the repo root via `LOG(msg)`/`LOG_ERR(msg)` macros.
