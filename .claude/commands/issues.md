---
description: Browse open GitHub issues for this repo
argument-hint: [search terms | label:refactor | assignee:@me]
---

List the open issues so we can pick what to work on. Filter: **$ARGUMENTS** (all open issues when
empty).

```bash
gh issue list --state open --limit 40 --json number,title,labels,assignees \
  --template '{{range .}}{{printf "%-5v" .number}} {{printf "%-10v" (index .labels 0).name}} {{.title}}{{"\n"}}{{end}}'
```

`GH_REPO` is set in `.claude/settings.json`, so `gh` resolves the repo despite the remote using the
`github.com-personal` SSH host alias — do not pass `-R` and do not parse the git remote yourself.

Pass `$ARGUMENTS` through as appropriate:
- bare words → `--search "<words>"`
- `label:x` → `--label x`
- `assignee:@me` → `--assignee @me`

## Presenting the results

Group by the type prefix in the title (`feat:`, `fix:`, `refactor:`, `docs:`, `test:`) — this repo's
issue templates prefill it, so the prefix is reliable. Lead with the count, then a compact table of
number / type / title. Do not dump raw JSON.

To show one issue in full: `gh issue view <n>`.

## After picking

Hand off to `/start-issue <number> [more...]` — that is what records the issue and cuts the branch.
Nothing under `src/`, `include/`, or `assets/` can be edited until it runs.

If `gh` reports it is not on PATH, the session's environment predates the install — restart Claude
Code. If it reports no authentication, run `gh auth login` (macOS: `brew install gh` first).
