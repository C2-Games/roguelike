---
description: Run the full local CI sweep (format, cppcheck, clang-tidy, build)
---

Run `scripts/ci-local.sh`, which mirrors `.github/workflows/ci.yml` exactly except for CodeQL.
Do not reimplement its checks — it already runs clang-format, cppcheck, clang-tidy and the build
against a `build/` tree with `compile_commands.json`.

```bash
python3 .claude/hooks/toolchain_run.py 'bash scripts/ci-local.sh'
```

That one line works for both developers: on macOS/Linux it runs the script directly; on Windows it
re-execs through WSL, where the C++ toolchain lives. Never write `wsl.exe` into a command — it
breaks the Mac/Linux developer. Use `--where` to see which environment was picked.

Success is the literal line `== ALL CHECKS PASSED ==`. Anything less is a failure — report the
failing section and fix it. The first run configures CMake and may take a few minutes; later runs
reuse `build/`.

This is broader than the per-edit `PostToolUse` hook, which runs only clang-format and cppcheck on
the single file just written. **clang-tidy and the build are checked only here**, so `/check` must
pass before a PR.

If it reports a missing tool, install the toolchain for the platform:

| Platform | Install |
|---|---|
| Debian/Ubuntu/WSL | `sudo apt install clang-format clang-tidy cppcheck cmake libncurses-dev` |
| macOS | `brew install clang-format llvm cppcheck cmake ncurses` |
