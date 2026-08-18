#!/usr/bin/env python3
"""Run a shell command in the repo's C++ toolchain, wherever that lives.

    python3 .claude/hooks/toolchain_run.py 'bash scripts/ci-local.sh'

On macOS/Linux this runs the command directly. On Windows it re-execs through
WSL, because the toolchain is not on the native PATH. Slash commands call this
instead of naming a shell, so a single command file works for both developers.

Output streams live (builds are slow and silence is unhelpful) and the child's
exit status is propagated. `--where` prints the resolved environment instead of
running anything.
"""

import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from _toolchain import needs_wsl, project_dir, toolchain_argv  # noqa: E402


def main(argv):
    if not argv or argv[0] in ("-h", "--help"):
        print(__doc__.strip())
        return 0

    if argv[0] == "--where":
        print("platform : {}".format(sys.platform))
        print("root     : {}".format(project_dir()))
        print("shell    : {}".format("WSL (wsl.exe)" if needs_wsl() else "native"))
        return 0

    # Accept either one quoted script or several words to join.
    script = argv[0] if len(argv) == 1 else " ".join(argv)

    cmd, run_cwd, error = toolchain_argv(script, project_dir())
    if error:
        sys.stderr.write(
            "toolchain_run: {}\n"
            "Install the C++ toolchain (clang-format, cppcheck, clang-tidy, cmake) "
            "or, on Windows, ensure WSL is available.\n".format(error)
        )
        return 127

    try:
        # No capture: stream straight through to the caller.
        return subprocess.call(cmd, cwd=run_cwd)
    except OSError as exc:
        sys.stderr.write("toolchain_run: {}\n".format(exc))
        return 127


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
