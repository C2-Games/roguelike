"""Shared helpers for the roguelike .claude hooks and commands.

This repo is worked on from two very different environments:

  * macOS / Linux -- the C++ toolchain (clang-format, cppcheck, clang-tidy,
    cmake) is on PATH, so shell work runs directly.
  * Windows -- the toolchain is NOT on PATH; it lives in WSL. Git Bash supplies
    a `bash` but none of the tools, so "is there a shell?" is the wrong question
    to ask. We decide on whether the *tools* resolve, and re-exec through
    `wsl.exe` when they do not.

Everything that needs a shell goes through `toolchain_argv()`, so the issue gate
and the slash commands make that decision exactly once and identically.
"""

import os
import shlex
import shutil
import subprocess
import sys

# Nothing here may hang the session waiting on a toolchain.
TOOL_TIMEOUT_SECONDS = 45


def project_dir():
    """Absolute path to the repo root."""
    env = os.environ.get("CLAUDE_PROJECT_DIR")
    if env:
        return os.path.abspath(env)
    # .claude/hooks/_toolchain.py -> repo root
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def to_wsl_path(win_path):
    """C:\\Users\\me\\repo -> /mnt/c/Users/me/repo. Pass through POSIX paths."""
    path = os.path.abspath(win_path).replace("\\", "/")
    if len(path) > 1 and path[1] == ":":
        return "/mnt/" + path[0].lower() + path[2:]
    return path


def _wsl_binary():
    return shutil.which("wsl.exe") or shutil.which("wsl")


def cpp_toolchain_is_native():
    """True when the C++ tools are on PATH here rather than only inside WSL.

    Cheap: a PATH lookup, no subprocess. Git Bash on Windows provides `bash` but
    not clang-format, so the presence of a shell is not the question worth
    asking -- the presence of the tools is.
    """
    return bool(shutil.which("clang-format") or shutil.which("cppcheck"))


def needs_wsl():
    """True only on Windows when the C++ tools are absent natively."""
    return sys.platform == "win32" and not cpp_toolchain_is_native()


def toolchain_argv(script, cwd=None):
    """Resolve `script` to an argv/cwd pair for the right shell.

    Returns (argv, run_cwd, error). On macOS/Linux this is plain
    `bash -c <script>`; on Windows without a native toolchain it becomes
    `wsl.exe -e bash -c "cd <translated cwd> && <script>"`.

    `bash -c`, not `-lc`: the tools live in /usr/bin and sourcing login profiles
    roughly doubles an already-expensive WSL spawn.
    """
    cwd = cwd or project_dir()

    if not needs_wsl():
        shell = shutil.which("bash") or shutil.which("sh")
        if not shell:
            return None, None, "no POSIX shell (bash/sh) found on PATH"
        return [shell, "-c", script], cwd, None

    wsl = _wsl_binary()
    if not wsl:
        return None, None, "no C++ toolchain on PATH and wsl.exe not found"
    inner = "cd {} && {}".format(shlex.quote(to_wsl_path(cwd)), script)
    return [wsl, "-e", "bash", "-c", inner], None, None


def git_branch():
    """Current branch name, or None if git is unavailable / detached HEAD."""
    try:
        done = subprocess.run(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"],
            cwd=project_dir(),
            capture_output=True,
            text=True,
            errors="replace",
            timeout=TOOL_TIMEOUT_SECONDS,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if done.returncode != 0:
        return None
    branch = done.stdout.strip()
    return branch or None


def read_payload():
    """Parse the hook payload from stdin. Returns {} when absent or malformed."""
    import json

    try:
        raw = sys.stdin.read()
    except (OSError, ValueError):
        return {}
    if not raw.strip():
        return {}
    try:
        return json.loads(raw)
    except ValueError:
        return {}


def target_path(payload):
    """The file a Write/Edit payload acts on, as an absolute path, or None."""
    tool_input = payload.get("tool_input") or {}
    response = payload.get("tool_response") or {}
    path = tool_input.get("file_path") or response.get("filePath")
    if not path:
        return None
    return os.path.abspath(path)


def relative_to_project(abs_path):
    """Project-relative POSIX path, or None if `abs_path` is outside the repo."""
    root = project_dir()
    try:
        rel = os.path.relpath(abs_path, root)
    except ValueError:  # different drive on Windows
        return None
    if rel.startswith(".."):
        return None
    return rel.replace("\\", "/")
