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
   `gh issue view <n> --json number,title,labels,body,milestone`
   (`GH_REPO` is set in `.claude/settings.json`, so the SSH-alias remote is handled.)
   If `gh` is not installed, say so once, accept the bare numbers, and ask the user for a
   one-line description to name the branch from.

1.5. **Load milestone context.** Take the *first* issue's milestone. If it has none, or `gh`
   errors, say so in one line and skip to step 2 — no blocking question for an edge case
   nothing was asked about. Otherwise fetch the full milestone set, both states:
   `gh issue list --state all --milestone "<name>" --json number,title,state,labels,body --limit 200`
   Closed issues in the set are implementation-pattern context (reused again in step 7); open
   ones (excluding the issue(s) being started) feed a lightweight relatedness pass —
   title/label/body overlap for "looks related enough to combine," and dependency language
   ("depends on," "blocks," "after #n") or same-subsystem overlap for "looks like a
   prerequisite."

   If candidates surface, present them via `AskUserQuestion` — one question per candidate (or
   grouped if several point the same direction) — options: "combine onto this branch," "do
   the other issue first instead," "proceed as-is." Do not continue past this step until
   answered. "Combine" adds that issue's number to the working set used from step 3 onward
   (branch name still derives from the *first* originally-requested issue); "do first" stops
   here and tells the user to run `/start-issue` on that issue instead.

   If nothing surfaces, say so in one sentence and continue automatically — never merge or
   reorder silently, but don't ask when there's nothing to flag.

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

   Keep the `<kebab-description>` short: at most 4 words, ideally 2-3. This holds even
   when the issue title is longer — pick the shortest phrase that still identifies the change,
   don't mechanically truncate the title.

3.5. **Decide whether to isolate this in a worktree.** Ask the user explicitly — do not decide
   silently — when either signal fires:
   - the request itself said "parallel", "worktree", or "isolated", or
   - `.claude/.current-issue` already exists and records a *different* branch than the one just
     derived (another issue is already active in this checkout).

   If neither fires, skip straight to step 4 as today — single-track is the default.

4. **Create the branch.**

   **No worktree (default):** from an up-to-date `main`:
   `git fetch origin && git switch -c <branch> origin/main`
   If the branch already exists, `git switch <branch>` instead.

   **Worktree (only if step 3.5 confirmed isolation):** call `EnterWorktree(name: <branch>)`
   instead. It creates the worktree under `.claude/worktrees/<branch>` off `origin/main` and
   switches the session into it. Confirm the branch it actually created — if it differs from
   `<branch>`, record the real name in step 5, don't force a rename.

5. **Write the record** to `.claude/.current-issue` (this file is gitignored):

   ```json
   {"issues": [108, 112], "branch": "feat/enemy-spawn-tables", "recorded": "<YYYY-MM-DD>"}
   ```

6. **Confirm** the issue numbers, the branch, and that the gate is now open.

7. **Enter plan mode** — call `EnterPlanMode`, then read the code the issue(s) touch and
   produce an implementation plan before writing anything. Do not start editing straight
   from the issue text: the issues here are terse and often understate which files move.
   Read the milestone issue list loaded in step 1.5 alongside the target issue's own text:
   closed siblings show established patterns/conventions for this area of the code, open
   ones flag upcoming work the plan shouldn't conflict with.

   **Exception — small changes skip formal plan mode.** A single-file edit that is purely
   documentation/comment text, or a genuine one-line fix, does not need `EnterPlanMode`: state
   the change in one sentence and proceed directly. Keep the bar genuinely small and
   single-file — anything touching `src/`/`include/` *behavior*, spanning multiple files, or
   otherwise non-trivial still requires the full flow below.

   **Ask before finalizing.** Ask clarifying implementation questions rather than guessing.
   Design and architecture decisions are the developer's to drive — they may hand you intent at
   the pseudo-code level — and the plan's job is to implement that precisely, not to invent
   architecture unprompted. Plans in this repo are made collaboratively, not unilaterally.

   **Check the plan against architecture before presenting it.** Once the plan's tasks are drafted,
   if any of them touch `src/` or `include/`, dispatch the `architecture-checker` agent
   (`.claude/agents/architecture-checker.md`) with the plan's task list before calling
   `ExitPlanMode`. It reads the plan against `ARCHITECTURE.md`'s six Coupling Rules and hands back
   any violation plus an architecture-preserving alternative — since `AskUserQuestion` is
   unavailable inside subagents, it hands the finding back rather than asking. If it reports a
   violation, ask the developer via `AskUserQuestion` which way to go: adopt the alternative,
   proceed as planned and note the `ARCHITECTURE.md` update this will require, or revise the plan —
   then fold the answer in before continuing. If it reports no violations, say so in one line and
   continue. Skip this step entirely when the plan touches neither directory (config-only work like
   doc or `.claude/` changes).

   The plan should name the specific files and symbols to change, follow the conventions in
   `CLAUDE.md` (trailing-underscore privates, `#ifndef` guards, forward declarations), and
   **must end with `/check`**. Nothing is formatted or analysed while you write — there is no
   write-time hook — so a plan without `/check` ships unverified code. Present it with
   `ExitPlanMode` for approval.

   **Break the plan into isolated tasks**, each written with this structure so parallel-dispatch
   eligibility is obvious at a glance instead of inferred from prose:

   ```
   ### Task <N>: <short title> — issue #<issue-number>
   **Subagent:** implementer
   **Depends on:** Task <M> | independent
   ```

   followed by the task's description. `Depends on: Task <M>` names the task whose output this one
   needs; `independent` means it can run in parallel with any other independent task. Once approved
   (`ExitPlanMode`), dispatch every task marked `independent` (relative to what's already landed) in
   parallel to the `implementer` agent (`.claude/agents/implementer.md`) — multiple `Agent` tool
   calls in a single message. Run a task with a `Depends on` marker only after that dependency's
   implementer call has returned and been folded in, one after another.

   **Mirror the breakdown as tracked tasks.** Before calling `ExitPlanMode`, call `TaskCreate`
   once per task (`subject` = the short title, `description` = the task's description) and follow
   with `TaskUpdate` to set `owner` to the subagent name and `addBlockedBy` to the IDs of the
   tasks it depends on — mapping directly from each task's `Depends on: Task <M>` marker. This
   keeps the breakdown as inspectable tool state (`TaskList`) in addition to the plan text. As
   each implementer dispatch returns and its output is folded in, mark that task `completed` via
   `TaskUpdate` before dispatching whichever task it was blocking. Skip this for a plan with only
   a single task — `TaskCreate`'s own guidance advises against use for one trivial task.

   **Include a `CLAUDE.md` step when the change earns one.** It is the map a future session
   reads before touching code, so it is updated as part of the work, not retrofitted after.
   Update it for: a new subsystem or file-layout change, ownership moving between types, a
   changed convention, or a placeholder becoming a real implementation. Skip it for renames,
   small refactors, and bugfixes — it is a map, not a changelog, and `git log` already records
   what changed. State either way in the plan so the reviewer can disagree.

   If the issues are several small independent renames on one branch, one plan covering all
   of them is fine — say which issue each step closes.

   **If implementation surfaces something that would change the approved plan** — not a small
   in-scope detail, but a real departure from what was approved — stop and explain the hurdle
   clearly, then ask the developer how to proceed. Do not improvise past it.

Do not commit or push — those are denied by `.claude/settings.json` and are the user's to run.
