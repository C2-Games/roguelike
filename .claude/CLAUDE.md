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

@.claude/ARCHITECTURE.md
