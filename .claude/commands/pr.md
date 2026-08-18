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

4. **Show what would ship.** `git status --short` and `git diff --stat origin/main...HEAD`.
   Point out any uncommitted work — the user still has to commit it themselves.

5. **Draft the title** in this repo's convention: `type: Sentence-case description`, where `type`
   matches the branch prefix (`feat`, `fix`, `refactor`, `docs`, `test`, `chore`). A scope is
   optional and occasionally used, e.g. `fix(build):`.

6. **Write the body** to `.claude/.pr-body.md` (gitignored), following
   `.github/PULL_REQUEST_TEMPLATE.md` exactly — Summary, Related Issues, Changes Made, Notes —
   with a `Closes #N` line for **every** recorded issue number.

7. **Print the handoff** and stop:

   ```
   ! git push -u origin <branch>
   ! gh pr create --title "<title>" --body-file .claude/.pr-body.md
   ```

   Tell the user to run these with the `!` prefix so they execute in their session.
