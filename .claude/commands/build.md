---
description: Build the game (debug by default, or release)
argument-hint: [debug|release]
---

Build target: **$ARGUMENTS** (default `debug` when empty).

- `debug` → `scripts/build-debug.sh` → `.build/debug/roguelike` (enables the FPS/coordinate
  debug overlay — `DebugLayer` is compiled in only when `NDEBUG` is undefined)
- `release` → `scripts/build-release.sh` → `.build/release/roguelike`

Pick the script from `$ARGUMENTS` and run **one** of these — an empty argument means `debug`:

```bash
# debug (default)
python3 .claude/hooks/toolchain_run.py 'bash scripts/build-debug.sh 2>&1 | tail -40'
```

```bash
# release
python3 .claude/hooks/toolchain_run.py 'bash scripts/build-release.sh 2>&1 | tail -40'
```

If `$ARGUMENTS` is anything other than `debug`, `release`, or empty, say so and stop rather
than guessing a target.

Runs natively on macOS/Linux and through WSL on Windows. Never write `wsl.exe` into a command —
it breaks the Mac/Linux developer.

Report compiler errors with the file and line. If the build succeeds, say so in one line — do not
paste the full CMake output.

**macOS note:** `scripts/build-*.sh` call plain `cmake -B ... -S .` with no `CMAKE_PREFIX_PATH`,
while `.github/workflows/ci.yml` passes `-DCMAKE_PREFIX_PATH=$(brew --prefix ncurses)` for
macos-latest. Homebrew keeps ncurses keg-only, so the bare script can fail to find it locally even
though CI is green. If that happens, configure explicitly:

```bash
cmake -B .build/debug -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$(brew --prefix ncurses)"
cmake --build .build/debug
```

That divergence between the scripts and CI is a real bug — open an issue for it rather than
patching around it silently.
