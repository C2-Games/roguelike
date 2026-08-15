# Roguelike

[![CI](https://github.com/C2-Games/roguelike/actions/workflows/ci.yml/badge.svg)](https://github.com/C2-Games/roguelike/actions/workflows/ci.yml)
[![Release](https://github.com/C2-Games/roguelike/actions/workflows/release.yml/badge.svg)](https://github.com/C2-Games/roguelike/actions/workflows/release.yml)
[![Latest release](https://img.shields.io/github/v/release/C2-Games/roguelike?include_prereleases)](https://github.com/C2-Games/roguelike/releases/latest)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

A terminal-based roguelike game written in C++17, rendered with ncurses. Navigate a procedurally-populated dungeon of hand-authored rooms, fight enemies that path toward you once spotted, and fire ranged weapons. Enemy positions persist when you leave a room and are restored when you return.

---

## Requirements

| Dependency                  | Minimum Version    | Notes                                  |
| ---------------------------- | ------------------ | --------------------------------------- |
| C++ compiler (GCC or Clang) | C++17 support      | GCC 7+ or Clang 5+                      |
| CMake                       | 3.16                | Build system                            |
| ncurses (wide-char)         | Any recent version | Terminal rendering library (needs the wide/`ncursesw` variant) |

**Installing ncurses (Debian/Ubuntu/WSL):**

```bash
sudo apt install libncurses-dev
```

**Installing ncurses (macOS):**

```bash
brew install ncurses
```

---

## Ways to play

### 1. Download a release (fastest)

Prebuilt binaries for Linux (x86_64) and macOS (Apple Silicon) are published on the [Releases page](https://github.com/C2-Games/roguelike/releases) by the `Release` workflow.

```bash
# example: replace <version> and pick the asset for your platform
tar -xzf roguelike-<version>-linux-x86_64.tar.gz -C roguelike-<version>
cd roguelike-<version>
./bin/roguelike
```

Each release includes a `.sha256` checksum file alongside the archive — verify with `sha256sum -c roguelike-<version>-linux-x86_64.sha256` before extracting if you want to confirm integrity.

### 2. Build from source

**Using the helper scripts (recommended):**

```bash
# release build -> .build/release/roguelike
./scripts/build-release.sh

# debug build (enables the FPS/position debug overlay) -> .build/debug/roguelike
./scripts/build-debug.sh
```

Both scripts `cd` to the repository root automatically, so they can be run from anywhere inside the repo.

**Manual CMake invocation:**

```bash
# release
cmake -B .build/release -S .
cmake --build .build/release

# debug
cmake -B .build/debug -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build .build/debug
```

The debug build enables the debug overlay (bottom-left of screen) showing live FPS and player coordinates.

---

## Run

```bash
# built from source
./.build/release/roguelike
# or, for a debug build
./.build/debug/roguelike

# extracted from a release archive
./bin/roguelike
```

Must be run from the directory containing `assets/` (the project root, or the root of an extracted release archive).

---

## Controls

| Key        | Action                                                        |
| ---------- | -------------------------------------------------------------- |
| `W` or `↑` | Move up                                                        |
| `S` or `↓` | Move down                                                      |
| `A` or `←` | Move left                                                      |
| `D` or `→` | Move right                                                     |
| `SPACE`    | Start the game (from the title prompt)                        |
| `SPACE`    | *(in-game)* Fire your equipped weapon in the direction you last moved |
| `Q`        | Quit the game                                                  |

**Moving onto a `+` door tile** that is linked to another room will teleport you to that room. Not all doors are linked — unlinked doors are cosmetic.

---

## Gameplay

- You start in Room 1 of 5. The HUD in the middle shows `Room X/5`, your HP, and your equipped weapon's name/damage/speed/range stats.
- Each room is randomly picked from the hand-authored templates under `assets/rooms/` (currently: chambered, cross hall, L-shape, maze, rectangular pillar hall, plain rectangular, ruins, and twin halls) and connected to the next in a chain via doors.
- You carry a ranged weapon (Basic by default) and fire projectiles with `SPACE` toward whichever direction you last moved. Projectiles travel until they hit an enemy, hit a wall, or run out of range.
- Enemies (`G`, `O`) wander until they spot you in their attack field of view, then actively path toward you. If you break line of sight, they'll keep heading toward your last known position for a few turns before giving up and going back to wandering.
- Taking damage from an enemy reduces your HP shown in the HUD above your character (`@`).
- The game ends when your HP reaches zero.
