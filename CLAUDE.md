# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Workflow (enforced, not advisory)

1. **No change without a GitHub issue.** Run `/start-issue <number>` before editing anything under
   `src/`, `include/`, `assets/`, or the build files. A `PreToolUse` hook denies those edits until
   an issue is recorded for the current branch, and always denies edits made on `main`. One branch
   may carry several issues — record them all.
2. **Format and static analysis run once per change, at `/check`.** Nothing is checked while you
   write — there is no write-time hook. `/check` applies `clang-format -i` over `src/`/`include/`,
   then runs `scripts/ci-local.sh` (cppcheck, clang-tidy, build). It is the required final step of
   every plan, and `/pr` will not prepare a PR over a failing sweep.
3. **Never `git commit` or `git push`.** Both are denied in `.claude/settings.json`; publishing is
   the user's call. `/pr` prepares the title and body and hands the commands back.
4. **No `TODO`/`FIXME` comments.** Pending work goes in a GitHub issue, never in the source. CI
   rejects `TODO`, `FIXME`, `XXX`, `HACK` and `TBD` in tracked sources; `/check` catches it first.
5. **This file is maintained with the code, not after it.** Every plan states whether the change
   earns an update here — yes for a new subsystem, a file-layout change, ownership moving between
   types, a changed convention, or a placeholder becoming real; no for renames, small refactors and
   bugfixes. As a backstop, a `Stop` hook (`.claude/hooks/doc_drift.py`) lists any workflow-defining
   file that changed while this one did not, once per distinct set of changes.

Commands: `/issues`, `/start-issue`, `/check`, `/build`, `/run`, `/pr`. The `cpp-style` skill
(`.claude/skills/cpp-style/SKILL.md`) carries the conventions no linter can check — naming, comment
voice, and where a Doxygen docstring is allowed to go — and applies to all C++ work here.

This repo is developed from both macOS/Linux and Windows. The C++ toolchain is on PATH on
macOS/Linux but lives in WSL on Windows, so **never write `wsl.exe` into a command or script**
— route shell work through `python3 .claude/hooks/toolchain_run.py '<command>'`, which picks
the right environment on both.

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
- Static analysis: `.clang-tidy` enables `bugprone-*`, `performance-*`, `readability-*` (minus `magic-numbers` and `identifier-length`). Also `cppcheck --enable=all` and CodeQL run in CI.
- Blame hygiene: `.git-blame-ignore-revs` lists formatting-only commits (the Allman/LF reformat).
  GitHub's web blame reads it automatically; **locally each clone needs it enabled once** with
  `git config blame.ignoreRevsFile .git-blame-ignore-revs`. Add future whitespace-only commits to it.
- CI (`.github/workflows/ci.yml`) builds Linux + macOS only — no Windows build/target.
- Releases (`.github/workflows/release.yml`) trigger when the `VERSION` file changes on `main`, tagging `v<VERSION>` and publishing Linux/macOS binaries.

## Architecture

Single-executable terminal roguelike (target `roguelike`), plain OOP (no ECS), ncurses rendering.

**Entry point & game loop**: `src/main.cpp` sets up ncurses and constructs `Game` (`include/core/game.h`, `src/core/game.cpp`), which owns essentially all game state and *is* the loop (a prior refactor removed a separate `Level`/stage abstraction — `Game::run()` now does `handleInput() → update() → render()` each frame, paced to `fps_`). `Game` owns: `GameServices` (holds the single `std::mt19937 rng`, injected by reference everywhere instead of using globals/statics, seeded with a fixed constant for reproducible runs), `Player`, `Level` (the *room graph* data structure, not a stage/scene), `GoalMapCache`, a `vector<unique_ptr<Projectile>>`, and `Renderer`. Enemies are not a `Game` member — they live per-`Room` (see below).

**World/map** (`include/world/map/`, `src/world/map/`):
- `Level` — room graph: `rooms_` map, `doorConnections_` keyed by `(roomID, doorPos)`, a per-room enemy spawn table (`getRoomEnemyConfig`). Built by `Level::Level(levelDir, services)` from a level config directory (`assets/levels/<name>/level.json` + `map.json` + `room_<id>.json`, parsed via `world/map/level_config.h`'s `loadLevelConfig`); door connections resolve `map.json`'s per-room compass-direction edges (`N`/`E`/`S`/`W`) against each loaded room's actual door tile on that edge.
- `Room` — fixed grid (`WIDTH=175`, `HEIGHT=50`) of `Tile`, loaded from hand-authored files in `assets/rooms/*.txt` via `Room::loadFromFile`: a `@key: value` header followed by an ASCII grid. Legend: `#`=Wall, `.`=Floor, `o`=Pillar, space=Void, `E`=enemy spawn, `L`=loot/item spawn, digits=numbered door tiles (doors are collected in numeric order into `doorPositions`). Also owns a `RoomEnemyState` member (`enemyState`) holding that room's live enemies, exposed via `enemies()`/`ensureEnemiesSpawned()`.

**Entities** (`include/entities/`, `src/entities/`): `Entity` is the abstract base (position, symbol, health, speed) for `Player` and `Enemy`. `Enemy::moveTowardPlayer` uses the goal-map/pathfinding system and tracks "chase memory" via `lastKnownPlayerPos_`. `EnemyCatalog` parses every file in `assets/enemies/` into a `(type, tier) → stats` lookup. Enemy spawning is `enemy_factory::rollForRoom` — **currently a placeholder**: each `E` spawn point has a hardcoded 50% chance to spawn, alternating hardcoded symbols `G`/`O`. `RoomEnemyState` (a member of each `Room`) holds that room's live enemies (HP/position persist across room transitions) and lazily rolls spawns on first visit via `ensureSpawned(room, services)`.

**In-progress work (this branch)**: `assets/levels/level_1/` and `EnemyCatalog`/`LevelConfig` are now fully wired — `Level` loads a real level directory and `EnemyCatalog` loads real enemy stat data — but `enemy_factory::rollForRoom` doesn't consume either yet; the hardcoded 50%/G-O placeholder is still what actually runs. When wiring real per-room spawn tables, `enemy_factory::rollForRoom` (`src/entities/enemy_factory.cpp`) and `RoomEnemyState::ensureSpawned` (`src/entities/room_enemy_state.cpp`) are where they plug in.

**Pathfinding** (`include/world/systems/`, `src/world/systems/`): `pathfinding::computeGoalMap` builds a Dijkstra/BFS distance-to-goal grid per room. `GoalMapCache` caches these keyed by `(roomID, goal)`, capped at 32 entries (clears entirely, not LRU, once full). `visibility::update` recomputes FoV/fog-of-war each frame; `FOV`/`ellipseFOV` (`include/entities/fov.h`) builds the offset-set shape, aspect-corrected for terminal cells.

**Rendering** (`include/render/`, `src/render/`): `RenderStack` is an abstract base wrapping one ncurses `WINDOW*`. `Renderer` holds layers in a z-ordered map and calls `compose()` each frame. Layers, added in `Game`'s constructor: `MapLayer` → `EntityLayer` (draws only what's in the player's FoV) → `HUDLayer` → `DebugLayer` (compiled in only when `NDEBUG` is not defined, i.e. debug builds).

**Conventions**: trailing underscore for private members; `#ifndef` include guards (no `#pragma once`); PascalCase classes, snake_case free-function namespaces (`enemy_factory::`, `visibility::`); `getX()`/`isX()` accessors; `unique_ptr` for polymorphic ownership, plain references for non-owning per-frame "context" structs (`MoveContext`, `ProjectileContext`) instead of globals; Doxygen-style `/** @brief */` header comments; heavy forward-declaration use to keep header coupling low.

**Logging**: `Logger::get()` singleton writes to `game.log`/`error.log` at the repo root via `LOG(msg)`/`LOG_ERR(msg)` macros.
