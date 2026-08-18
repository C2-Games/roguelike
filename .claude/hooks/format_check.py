#!/usr/bin/env python3
"""PostToolUse hook: clang-format + cppcheck every C++ file that gets written.

CI (.github/workflows/ci.yml) only gates these on a pull request to main, so
failures otherwise surface minutes after a push. This runs them the moment the
file is written instead.

clang-format is applied in place -- formatting has exactly one correct answer,
so fixing beats reporting. cppcheck needs judgement, so its findings are handed
back as blocking feedback for Claude to address.

Flags mirror scripts/ci-local.sh exactly, so passing here means passing CI.
clang-tidy is deliberately left to /check: it needs build/compile_commands.json
and takes seconds per file, which is too slow for a per-edit hook.

Both tools run in a single shell invocation -- on Windows each WSL spawn costs
over a second, and this hook fires on every edit.
"""

import hashlib
import json
import os
import shlex
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from _toolchain import (  # noqa: E402
    project_dir,
    read_payload,
    relative_to_project,
    run_script,
    target_path,
)

CPP_SUFFIXES = (".cpp", ".h", ".hpp")
CHECKED_ROOTS = ("src/", "include/")

NO_TOOLS = "__NO_TOOLS__"
CF_FAILED = "__CF_FAILED__"

# Mirrors the cppcheck step in scripts/ci-local.sh, plus --quiet and a
# checkersReport suppression so a clean file produces no output at all.
SCRIPT = r"""
have_cf=0; have_cc=0
command -v clang-format >/dev/null 2>&1 && have_cf=1
command -v cppcheck     >/dev/null 2>&1 && have_cc=1
if [ "$have_cf" = 0 ] && [ "$have_cc" = 0 ]; then echo {no_tools}; exit 0; fi
if [ "$have_cf" = 1 ]; then
  clang-format -i -style=file {f} || echo {cf_failed} >&2
fi
if [ "$have_cc" = 1 ]; then
  cppcheck --enable=all --std=c++20 --error-exitcode=1 \
    --suppress=missingIncludeSystem --suppress=unusedFunction \
    --suppress=checkersReport --quiet -I include {f} || exit 1
fi
exit 0
"""


def emit(obj):
    json.dump(obj, sys.stdout)
    sys.stdout.write("\n")


def digest(path):
    with open(path, "rb") as handle:
        return hashlib.sha1(handle.read()).hexdigest()


def main():
    payload = read_payload()
    abs_path = target_path(payload)
    if not abs_path or not abs_path.endswith(CPP_SUFFIXES):
        return 0

    rel = relative_to_project(abs_path)
    if not rel or not rel.startswith(CHECKED_ROOTS):
        return 0
    if not os.path.isfile(abs_path):
        return 0

    before = digest(abs_path)
    script = SCRIPT.format(
        f=shlex.quote(rel), no_tools=NO_TOOLS, cf_failed=CF_FAILED
    )
    code, out, err = run_script(script, cwd=project_dir())

    # A missing toolchain must never block editing.
    if code == 127 or NO_TOOLS in out:
        emit(
            {
                "systemMessage": (
                    "Skipped format/cppcheck for {}: no C++ toolchain found "
                    "natively or in WSL.".format(rel)
                ),
                "suppressOutput": True,
            }
        )
        return 0

    if code != 0:
        findings = "\n".join(part for part in (err.strip(), out.strip()) if part)
        emit(
            {
                "decision": "block",
                "reason": (
                    "cppcheck failed on {} -- this is a CI gate, fix it before "
                    "moving on:\n\n{}".format(rel, findings)
                ),
            }
        )
        return 0

    notes = []
    if CF_FAILED in err:
        notes.append("clang-format errored on {}".format(rel))
    elif digest(abs_path) != before:
        notes.append("clang-format reformatted {}".format(rel))
    if notes:
        emit({"systemMessage": "; ".join(notes), "suppressOutput": True})
    return 0


if __name__ == "__main__":
    sys.exit(main())
