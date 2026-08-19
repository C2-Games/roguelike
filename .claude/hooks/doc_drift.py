#!/usr/bin/env python3
"""Stop hook: flag workflow changes that CLAUDE.md may no longer describe.

CLAUDE.md makes concrete, checkable claims -- which hooks are wired, which
commands exist, how formatting is enforced, what the build produces. Every one
of those claims has a file behind it, so when that file changes and CLAUDE.md
does not, the documentation is a candidate for being wrong.

A hook cannot judge whether an update is actually *needed* -- that takes reading
both sides. So this only supplies the signal: which watched files changed, and
whether CLAUDE.md changed with them. The model decides.

Runs on Stop rather than PostToolUse deliberately: per-write checking was
removed from this repo (see WORKFLOW.md rule 2) because each invocation costs a
WSL spawn on Windows. This is pure git, no toolchain, and fires once per turn.

Fires at most once per distinct drift set, and not at all once CLAUDE.md has
been edited since the last prompt -- so answering it, either by updating the
file or by deciding no update is warranted, does not loop on the next stop.

CLAUDE.md is hashed directly rather than looked for in git output. A global
gitignore can exclude it (this repo's owner has exactly that), and an ignored
file never appears in `git status` or a branch diff, which would leave the
prompt unanswerable by editing the file.
"""

import hashlib
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from _toolchain import project_dir  # noqa: E402

ACK_NAME = os.path.join(".claude", ".doc-drift-ack")
# Literal forward-slash string, not os.path.join: changed_files() below builds its paths
# from `git diff`/`git status`, which always emit forward slashes even on Windows, and
# `DOC in paths` (see main()) needs to match that exactly. os.path.join would silently
# break the match on Windows and re-prompt on every CLAUDE.md edit.
DOC = ".claude/CLAUDE.md"

# Each entry backs a specific claim in CLAUDE.md. Adding a path here without a
# corresponding claim just produces noise.
WATCHED = (
    ".claude/settings.json",   # rules 1-3: which hooks are wired, what is denied
    ".claude/commands/",       # the command list
    ".claude/hooks/",          # hook behaviour
    ".claude/skills/",         # the cpp-style pointer
    ".claude/agents/",         # the implementer/reviewer agents
    "scripts/",                # build & local CI sections
    ".github/workflows/",      # the CI section
    "CMakeLists.txt",          # build & dependency notes
    ".clang-format",           # lint/format section
    ".clang-tidy",
    "src/",                    # Architecture section -- see reviewer.md
    "include/",
)


def git(*args):
    """Run a git command in the repo, returning stdout or '' on failure."""
    try:
        done = subprocess.run(
            ("git",) + args,
            cwd=project_dir(),
            capture_output=True,
            text=True,
            errors="replace",
            timeout=15,
        )
    except (OSError, subprocess.SubprocessError):
        return ""
    return done.stdout if done.returncode == 0 else ""


def changed_files():
    """Every path this branch touches: committed since main, plus uncommitted."""
    paths = set()
    for line in git("diff", "--name-only", "origin/main...HEAD").splitlines():
        paths.add(line.strip())
    # --porcelain columns are fixed-width; the path starts at column 3.
    for line in git("status", "--porcelain").splitlines():
        entry = line[3:].strip()
        if entry:
            paths.add(entry.split(" -> ")[-1])
    return {p for p in paths if p}


def ack_path():
    return os.path.join(project_dir(), ACK_NAME)


def doc_hash():
    """Content hash of CLAUDE.md, or '' when it is absent."""
    try:
        with open(os.path.join(project_dir(), DOC), "rb") as handle:
            return hashlib.sha1(handle.read()).hexdigest()
    except OSError:
        return ""


def read_ack():
    """(drift digest, CLAUDE.md hash) recorded the last time this fired."""
    try:
        with open(ack_path(), "r", encoding="utf-8") as handle:
            parts = handle.read().split()
    except OSError:
        return "", ""
    parts += ["", ""]
    return parts[0], parts[1]


def write_ack(digest, doc):
    try:
        with open(ack_path(), "w", encoding="utf-8") as handle:
            handle.write("{} {}\n".format(digest, doc))
    except OSError:
        pass  # an unwritable ack file must not break the session.


def main():
    paths = changed_files()
    if not paths or DOC in paths:
        return 0

    drifted = sorted(p for p in paths if p.startswith(WATCHED))
    if not drifted:
        return 0

    digest = hashlib.sha1("\n".join(drifted).encode("utf-8")).hexdigest()
    current_doc = doc_hash()
    acked_digest, acked_doc = read_ack()

    # Either this exact set was already raised, or CLAUDE.md has been edited
    # since it was raised. Both mean the question has been answered.
    answered = acked_digest == digest or (acked_doc and current_doc != acked_doc)
    write_ack(digest, current_doc)
    if answered:
        return 0

    listing = "\n".join("  - {}".format(p) for p in drifted)
    json.dump(
        {
            "decision": "block",
            "reason": (
                "These files define workflow that CLAUDE.md documents, and they "
                "changed on this branch while CLAUDE.md did not:\n\n{}\n\n"
                "Read CLAUDE.md and confirm it still describes the repo "
                "accurately -- the hooks it says are wired, the commands it "
                "lists, how formatting is enforced, what the build produces. "
                "Update it if any claim is now wrong. If everything still "
                "holds, say so in one line and stop; this will not ask "
                "again for the same set of changes.".format(listing)
            ),
        },
        sys.stdout,
    )
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
