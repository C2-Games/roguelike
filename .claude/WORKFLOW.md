# Working in this repo

Three rules hold here, and all three are enforced mechanically rather than by convention.

## 1. No change without a GitHub issue

Every change traces to an issue. Issues are opened from the templates at
`.github/ISSUE_TEMPLATE/` — `blank_issues_enabled: false`, so there is no freeform option, and each
template prefills a Conventional-Commit title (`feat:`, `fix:`, `refactor:`, `docs:`, `test:`).

**Enforcement:** `.claude/hooks/issue_gate.py` runs as a `PreToolUse` hook on `Edit|Write` and
*denies* the edit unless `.claude/.current-issue` records an issue for the current branch. It also
refuses any edit made while on `main`, and refuses a record that was written for a different branch.

Run `/start-issue <number> [more...]` to open the gate. One branch may carry several issues — that
is normal here; record them all.

Deliberately not gated: paths outside the repo, and `.claude/**` (otherwise this configuration
could never be repaired). Known gap: the gate covers `Edit`/`Write`, not shell redirection through
`Bash`.

## 2. Format and static analysis are checked at write time, not at PR time

`.github/workflows/ci.yml` only gates on `pull_request` to `main`, so CI failures otherwise surface
long after the fact.

**Enforcement:** `.claude/hooks/format_check.py` runs as a `PostToolUse` hook on every `.cpp`,
`.h`, or `.hpp` file written under `src/` or `include/`:

- **clang-format** is applied *in place* (`-i -style=file`). Formatting has one correct answer, so
  it is fixed rather than reported.
- **cppcheck** runs with the exact flags from `scripts/ci-local.sh`; findings come back as blocking
  feedback to fix before moving on.

clang-tidy and the build are **not** in this hook — clang-tidy needs `build/compile_commands.json`
and takes seconds per file. Both are covered by `/check`, which must pass before a PR.

## 3. Claude never commits or pushes

`git commit` and `git push` are in `permissions.deny` in `.claude/settings.json`, for both the Bash
and PowerShell tools. Staging, branching, diff and log remain available. `/pr` prepares the title
and body and hands the push/create commands back to you.

---

## Lifecycle

```
/issues  ->  /start-issue N  ->  plan (approve)  ->  edit (hooks run per write)  ->  /check  ->  /pr  ->  you push
```

## Common commands

| Command | What it does |
|---|---|
| `/issues [filter]` | List open GitHub issues to pick from (`gh issue list`, repo resolved via `GH_REPO`) |
| `/start-issue <n> [n...]` | Fetch the issue(s), cut a `<type>/<kebab-description>` branch off `main`, record `.claude/.current-issue`, then enter plan mode |
| `/check` | Full local CI sweep via `scripts/ci-local.sh` — format, cppcheck, clang-tidy, build |
| `/build [debug\|release]` | `scripts/build-debug.sh` / `scripts/build-release.sh` |
| `/run` | Build debug, run from the repo root, report `game.log` and `error.log` |
| `/pr` | Verify issue + `/check`, write `.claude/.pr-body.md`, print the push/create commands for you |

## Two environments — never hard-code one

This repo is worked on from macOS/Linux **and** from Windows, and the two differ in one way that
matters: on macOS/Linux the C++ toolchain (`clang-format`, `cppcheck`, `clang-tidy`, `cmake`) is on
PATH, while on Windows it is not — it lives in WSL. Git Bash supplies a `bash` on Windows but none
of the tools, so the presence of a shell is not a useful probe.

`.claude/hooks/_toolchain.py` makes that decision once, in `toolchain_argv()`, based on whether the
*tools* resolve. Hooks and slash commands both go through it, so there is a single code path.

**Never write `wsl.exe` into a command file** — it breaks the macOS/Linux developer. Route shell
work through the runner instead:

```bash
python3 .claude/hooks/toolchain_run.py 'bash scripts/ci-local.sh'
```

It runs the command directly on macOS/Linux and re-execs through WSL on Windows, streaming output
either way and propagating the exit status. `--where` prints which environment was picked.

Two portability rules that have already bitten:

- Each `wsl.exe` spawn costs over a second, so hooks batch all their shell work into a **single**
  invocation. The naive per-tool version took 6.7s per edit; batching brought it to ~0.4s.
- `script(1)` takes different arguments on GNU (`script -qc CMD /dev/null`) and BSD/macOS
  (`script -q /dev/null CMD`). `/run` detects the flavour rather than assuming.

Installing the toolchain:

| Platform | Install |
|---|---|
| Debian/Ubuntu/WSL | `sudo apt install clang-format clang-tidy cppcheck cmake libncurses-dev` |
| macOS | `brew install clang-format llvm cppcheck cmake ncurses` |

On macOS, Homebrew keeps ncurses keg-only and `scripts/build-*.sh` do not pass
`-DCMAKE_PREFIX_PATH="$(brew --prefix ncurses)"` the way `ci.yml` does — see `/build`.

## Branch and commit conventions

- Branches: `<type>/<kebab-description>` — `feat/`, `fix/`, `refactor/`, `chore/`, `docs/`, `test/`.
  Use `feat/`, not `feature/`; both appear in history but `feat/` is what recent work uses.
- Commits/PR titles: `type: Sentence-case description`. Squash-merge appends the PR number, so
  history reads `feat: Level config loading (#107)`. Optional scope: `fix(build):`.
- Issue linkage lives in the PR body's `Closes #N`, not in the commit message.
