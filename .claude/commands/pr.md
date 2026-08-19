---
description: Prepare a PR body and hand the push/create commands back to the user
---

**This command never touches the remote.** `git push` and `git commit` are denied in
`.claude/settings.json` — publishing is the user's call, always.

## Steps

1. **Require an issue.** Read `.claude/.current-issue`. If it is missing, stop and tell the user
   to run `/start-issue <number>` — a PR without a linked issue is not allowed here.

2. **Verify the branch.** Confirm `git rev-parse --abbrev-ref HEAD` matches the recorded branch,
   and that it is not `main`.

3. **Run `/check`.** Do not prepare a PR over a failing sweep. If it fails, report and stop.
   `/check` also dispatches a read-only `reviewer` agent pass whenever the branch's diff touches
   `src/`/`include/`, so a failing review counts as a failing sweep here too.

4. **Show what would ship.** `git status --short` and `git diff --stat origin/main...HEAD`.

   Read these two together: the diffstat shows only *committed* work, so uncommitted changes are
   invisible in it. If `git status` is not clean, say so loudly — pushing at that point ships the
   previous commit and silently omits the session's work. Name any file that should **not** go in
   (unrelated untracked assets, scratch files) so it is left out of the `git add`.

5. **Draft the title** in this repo's convention: `type: Sentence-case description`, where `type`
   matches the branch prefix (`feat`, `fix`, `refactor`, `docs`, `test`, `chore`). A scope is
   optional and occasionally used, e.g. `fix(build):`.

6. **Write the body** to `.claude/.pr-body.md` (gitignored), following
   `.github/PULL_REQUEST_TEMPLATE.md` exactly — Summary, Related Issues, Changes Made, Notes —
   with a `Closes #N` line for **every** recorded issue number.

7. **Print the handoff** and stop:

   ```
   ! git add <the files from step 4>
   ! git commit -m "<type>: <Sentence-case description>"
   ! git push -u origin <branch>
   ! gh pr create --title "<title>" --body-file .claude/.pr-body.md
   ```

   Include the `add`/`commit` lines whenever step 4 showed uncommitted work — omitting them is
   how a push ends up shipping the previous commit. Draft a real commit message; do not leave a
   placeholder. Tell the user to run these with the `!` prefix so they execute in their session.

   `git commit` and `git push` are denied to you, so these are the user's to run — which is the
   point. They are also the last human review before anything leaves the machine.
