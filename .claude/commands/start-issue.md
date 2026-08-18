---
description: Record the GitHub issue(s) for this work and cut the branch
argument-hint: <issue-number> [more-issue-numbers...]
---

Start work on issue(s): **$ARGUMENTS**

Nothing in `src/`, `include/`, `assets/`, or the build files can be edited until this
command has recorded an issue — the `PreToolUse` gate in `.claude/hooks/issue_gate.py`
denies those edits outright. Run this first, every time.

## Steps

1. **Fetch each issue.** For every number in `$ARGUMENTS`:
   `gh issue view <n> --json number,title,labels,body`
   (`GH_REPO` is set in `.claude/settings.json`, so the SSH-alias remote is handled.)
   If `gh` is not installed, say so once, accept the bare numbers, and ask the user for a
   one-line description to name the branch from.

2. **Confirm scope.** Summarise each issue in a sentence. If the issues do not plausibly
   belong on one branch, say so and ask before continuing.

3. **Derive the branch name** from the *first* issue's title, matching this repo's
   convention `<type>/<kebab-description>`:

   | Issue title prefix | Branch prefix |
   |---|---|
   | `feat:` | `feat/` |
   | `fix:` | `fix/` |
   | `refactor:` | `refactor/` |
   | `docs:` | `docs/` |
   | `test:` | `test/` |
   | anything else | `chore/` |

   Use `feat/`, never `feature/` — both exist in history but `feat/` is what recent work uses.

4. **Create the branch** from an up-to-date `main`:
   `git fetch origin && git switch -c <branch> origin/main`
   If the branch already exists, `git switch <branch>` instead.

5. **Write the record** to `.claude/.current-issue` (this file is gitignored):

   ```json
   {"issues": [108, 112], "branch": "feat/enemy-spawn-tables", "recorded": "<YYYY-MM-DD>"}
   ```

6. **Confirm** the issue numbers, the branch, and that the gate is now open.

7. **Enter plan mode** — call `EnterPlanMode`, then read the code the issue(s) touch and
   produce an implementation plan before writing anything. Do not start editing straight
   from the issue text: the issues here are terse and often understate which files move.

   The plan should name the specific files and symbols to change, follow the conventions in
   `CLAUDE.md` (trailing-underscore privates, `#ifndef` guards, forward declarations), and
   **must end with `/check`**. Nothing is formatted or analysed while you write — there is no
   write-time hook — so a plan without `/check` ships unverified code. Present it with
   `ExitPlanMode` for approval.

   If the issues are several small independent renames on one branch, one plan covering all
   of them is fine — say which issue each step closes.

Do not commit or push — those are denied by `.claude/settings.json` and are the user's to run.
