---
description: File a new GitHub issue, with milestone assignment
argument-hint: <freeform description of the issue>
---

File a new GitHub issue for: **$ARGUMENTS**

`GH_REPO` is set in `.claude/settings.json`, so `gh` resolves the repo despite the remote using
the `github.com-personal` SSH host alias — do not pass `-R` and do not parse the git remote
yourself.

This command only files the issue — it does not open the edit gate. Run `/start-issue <n>`
afterward to start coding against it.

This same draft-and-confirm flow also triggers without the user typing `/new-issue`: when a
change is requested directly, with no issue on record yet and no `/start-issue` run, Claude
follows these exact steps — match template, draft title/body, fetch milestones, confirm with the
user via `AskUserQuestion`, then create — before proceeding. Confirmation is required either way;
nothing here becomes more automatic.

## Steps

1. **Determine type and draft.** Match `$ARGUMENTS` to the closest issue template:

   | Sounds like | Type | Template |
   |---|---|---|
   | a defect, unintended behavior | `fix` | `.github/ISSUE_TEMPLATE/bug.yml` |
   | new feature or gameplay/system improvement | `feat` | `.github/ISSUE_TEMPLATE/feature.yml` |
   | internal structure/readability/maintainability | `refactor` | `.github/ISSUE_TEMPLATE/refactor.yml` |
   | documentation | `docs` | `.github/ISSUE_TEMPLATE/docs.yml` |
   | automated/manual testing | `test` | `.github/ISSUE_TEMPLATE/test.yml` |

   Draft a title (`<type>: <description>`, Conventional-Commit style) and a body matching that
   template's fields exactly (e.g. `feature.yml`'s Summary/Motivation/Possible Implementation).
   Label is the type; assignees are always `calvinmcelvain`, `Collin-McElvain` — every template
   fixes both.

2. **Fetch open milestones:**
   ```bash
   gh api repos/$GH_REPO/milestones --method GET -f state=open --jq '.[] | "\(.number)\t\(.title)"'
   ```

3. **Ask which milestone** with `AskUserQuestion`: list the open milestones from step 2 as
   options, plus a "New milestone" option. Only create a new one if the user explicitly picks
   that option and names a title (a version string) — never invent one silently. Create it with:
   ```bash
   gh api repos/$GH_REPO/milestones -f title="<title>"
   ```

4. **Show the draft** — title, type/label, body, chosen milestone — and wait for explicit
   confirmation before creating anything. Nothing here is reversible the way a local edit is.

5. **Create the issue:**
   ```bash
   gh issue create --title "<title>" --label <type> \
     --assignee calvinmcelvain,Collin-McElvain --body "<body>"
   ```
   Then set the milestone from step 3, using the issue number `gh issue create` returns:
   ```bash
   gh issue edit <n> --milestone "<title>"
   ```

6. **Report** the issue URL, number, and assigned milestone. Remind the user to run
   `/start-issue <n>` when they're ready to start coding against it.
