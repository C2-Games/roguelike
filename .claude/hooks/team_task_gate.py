#!/usr/bin/env python3
"""Blocking hook for agent-team task events: plan structure and issue record.

Wired to two events, both of which block on exit status 2 with the reason on
stderr:

  * TaskCreated -- denies a task with no owner, or whose subject/description
    carries no `issue #<n>` reference. Team tasks mirror the plan's per-issue
    breakdown, so a task that names no issue has no place to trace back to.
  * TaskCompleted -- denies completion while no issue is on record, or while
    the record was made on a different branch than HEAD. A task cannot be
    closed out against work the issue gate would refuse to edit.

Everything else is allowed with exit 0.
"""

import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from _toolchain import (  # noqa: E402
    git_branch,
    project_dir,
    read_payload,
)

RECORD_NAME = os.path.join(".claude", ".current-issue")
ISSUE_REF = re.compile(r"issue #\d+", re.IGNORECASE)


def load_record():
    try:
        with open(
            os.path.join(project_dir(), RECORD_NAME), "r", encoding="utf-8"
        ) as handle:
            record = json.load(handle)
    except (OSError, ValueError):
        return None
    if not isinstance(record, dict) or not record.get("issues"):
        return None
    return record


def first_present(payload, *keys):
    """First non-empty value among `keys`, so the schema can drift safely."""
    for key in keys:
        value = payload.get(key)
        if value:
            return value
    return ""


def deny(reason):
    sys.stderr.write(reason + "\n")
    return 2


def task_created(payload):
    owner = first_present(payload, "owner", "task_owner")
    subject = first_present(payload, "task_subject", "subject")
    description = first_present(payload, "task_description", "description")

    if not owner:
        return deny(
            "Team task has no owner. Plan tasks are assigned to a subagent "
            "(owner = implementer, reviewer, ...); set one before creating it."
        )
    if not ISSUE_REF.search("{} {}".format(subject, description)):
        return deny(
            "Team task names no issue. Put `issue #<n>` in its subject or "
            "description so it traces to the plan's issue."
        )
    return 0


def task_completed():
    record = load_record()
    if not record:
        return deny(
            "No GitHub issue on record, so this task cannot be completed. Run "
            "`/start-issue <number>` first."
        )

    branch = git_branch()
    recorded_branch = record.get("branch")
    if recorded_branch != branch:
        return deny(
            "The issue record is stale: it was made for branch `{recorded}`, "
            "but HEAD is `{actual}`. Run `/start-issue <number>` to record "
            "the issue(s) for this branch.".format(
                recorded=recorded_branch, actual=branch or "unknown"
            )
        )
    return 0


def main():
    payload = read_payload()
    event = payload.get("hook_event_name")

    if event == "TaskCreated":
        return task_created(payload)
    if event == "TaskCompleted":
        return task_completed()
    return 0


if __name__ == "__main__":
    sys.exit(main())
