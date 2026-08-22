---
description: File a new GitHub issue, with milestone assignment
argument-hint: <freeform description of the issue>
---

Dispatch the `issue-drafter` agent (`.claude/agents/issue-drafter.md`) with the request:
**$ARGUMENTS**

This command only files the issue — it does not open the edit gate. Run `/start-issue <n>`
afterward to start coding against it.

The same dispatch also triggers without the user typing `/new-issue`: when a change is requested
directly, with no issue on record yet and no `/start-issue` run, dispatch `issue-drafter` with that
request before proceeding — it handles template matching, follow-up questions, and confirmation on
its own.
