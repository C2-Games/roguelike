#!/usr/bin/env bash
# Runs the same checks as .github/workflows/ci.yml, locally.
# Usage: ./scripts/ci-local.sh
#
# Fails fast on the first check that errors. Uses build/ (not .build/debug or
# .build/release) so compile_commands.json matches what ci.yml expects.

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

section() { printf '\n== %s ==\n' "$1"; }

# Full run transcript, including clang-tidy's per-header findings that are
# kept off the console below. build/ is gitignored.
mkdir -p build
LOG_FILE="build/ci-local.log"
: > "$LOG_FILE"
# fd 3: log file only -- lets a step write findings to the log without also
# printing them to the console.
exec 3>>"$LOG_FILE"
# fd 1/2: console + log, for everything else in this script.
exec > >(tee -a "$LOG_FILE") 2>&1

# clang-tidy needs compile_commands.json. Configure once if missing.
if [[ ! -f build/compile_commands.json ]]; then
  section "cmake configure (with compile_commands)"
  cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null
fi

section "clang-format"
find src include -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
  -print0 | xargs -0 clang-format --dry-run --Werror -style=file

section "cppcheck"
# checkersReport/normalCheckLevelMaxBranches are information-severity output that
# newer cppcheck emits under --enable=all; --error-exitcode=1 counts them as
# failures. The versions differ by environment -- CI runs 2.13, local installs
# are newer -- and a suppression the running version never emits is itself
# reported as unmatchedSuppression, so that must be suppressed too or this fails
# on whichever version you are not testing.
# Keep these flags in sync with .github/workflows/ci.yml.
cppcheck --enable=all --std=c++20 --error-exitcode=1 \
  --suppress=missingIncludeSystem --suppress=unusedFunction \
  --suppress=checkersReport --suppress=normalCheckLevelMaxBranches \
  --suppress=unmatchedSuppression \
  -I include src/ include/

section "clang-tidy"
# -n1 -P forces one clang-tidy process per file, run in parallel across cores --
# xargs otherwise batches every file into a single invocation, making -P a
# no-op. --quiet drops the "N warnings generated / Suppressed N" boilerplate
# clang-tidy prints per file for suppressed non-user-code (mainly the
# nlohmann/json.hpp single-header, which alone accounts for ~44k suppressed
# diagnostics per translation unit).
# Requires clang-tidy 19+ (versioned binary, not the unversioned `clang-tidy`
# symlink) for .clang-tidy's ExcludeHeaderFilterRegex, which -- combined with
# HeaderFilterRegex -- surfaces this project's own include/ header warnings
# while still excluding nlohmann/json.hpp and system ncurses headers. Now that
# real findings print, that's a lot of console noise for warnings nothing here
# fails the build on -- write them to the log (fd 3) instead of the console
# for now, until they're triaged.
# getconf _NPROCESSORS_ONLN is portable across Linux and macOS; nproc is
# GNU-coreutils-only and absent on a stock macOS toolchain.
find src -name "*.cpp" -print0 |
  xargs -0 -n1 -P "$(getconf _NPROCESSORS_ONLN)" clang-tidy-19 -p build --quiet >&3 2>&3
warning_count=$(grep -c "warning:" "$LOG_FILE" || true)
printf 'clang-tidy: %s warning(s) found -- see %s\n' "$warning_count" "$LOG_FILE"

section "build"
cmake --build build --parallel

printf '\n== ALL CHECKS PASSED ==\n'
