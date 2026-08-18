---
description: Run the full local CI sweep (format, cppcheck, clang-tidy, build)
---

This is the **only** place formatting and static analysis happen. There is no write-time hook —
nothing has been checked until this runs, so it is the final step of every plan and `/pr` refuses
to prepare a PR over a failing sweep.

## 1. Apply formatting

Formatting has exactly one correct answer, so fix it rather than report it. This must run *before*
the sweep: `scripts/ci-local.sh` checks formatting with `--dry-run --Werror` and `set -euo pipefail`
aborts the whole run on a failure, so an unformatted file would stop cppcheck, clang-tidy and the
build from ever executing.

```bash
python3 .claude/hooks/toolchain_run.py 'find src include -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -print0 | xargs -0 clang-format -i -style=file'
```

It edits in place and prints nothing on success. Mention it in passing if it changed files; do not
paste diffs.

## 2. Run the sweep

`scripts/ci-local.sh` mirrors `.github/workflows/ci.yml` exactly except for CodeQL. Do not
reimplement its checks — it already runs clang-format, cppcheck, clang-tidy and the build against a
`build/` tree with `compile_commands.json`.

```bash
python3 .claude/hooks/toolchain_run.py 'bash scripts/ci-local.sh'
```

Both commands work for either developer: on macOS/Linux they run directly; on Windows they re-exec
through WSL, where the C++ toolchain lives. Never write `wsl.exe` into a command — it breaks the
Mac/Linux developer. Use `--where` to see which environment was picked.

Success is the literal line `== ALL CHECKS PASSED ==`. Anything less is a failure — report the
failing section and fix it. The first run configures CMake and may take a few minutes; later runs
reuse `build/`.

If it reports a missing tool, install the toolchain for the platform:

| Platform | Install |
|---|---|
| Debian/Ubuntu/WSL | `sudo apt install clang-format clang-tidy cppcheck cmake libncurses-dev` |
| macOS | `brew install clang-format llvm cppcheck cmake ncurses` |
