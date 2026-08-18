#!/usr/bin/env bash
# Runs the same checks as .github/workflows/ci.yml, locally.
# Usage: ./scripts/ci-local.sh
#
# Fails fast on the first check that errors. Uses build/ (not .build/debug or
# .build/release) so compile_commands.json matches what ci.yml expects.

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

section() { printf '\n== %s ==\n' "$1"; }

# Pending work belongs in the issue tracker, not in a comment nobody queries.
# Runs first: it needs no toolchain, so it fails before anything expensive.
#
# The `if` inverts the exit sense -- git grep exits 0 on a match and 1 when
# clean, the opposite of every other check here, and a bare call would abort
# this script under `set -e` on a *clean* tree.
#
# -w matches whole words (TODOLIST is not a hit). git grep, not grep -r, so
# only tracked files are searched and build/_deps third-party code needs no
# exclusion. This file excludes itself: the pattern below contains the very
# words it searches for, and scripts/ is inside the search path.
# Keep the pattern and pathspec in sync with .github/workflows/ci.yml.
section "todo-scan"
todo_paths=(src include scripts .claude ':!*.md' ':!*.txt' ':!scripts/ci-local.sh')
if git grep -nwE 'TODO|FIXME|XXX|HACK|TBD' -- "${todo_paths[@]}"; then
  echo "Pending work belongs in a GitHub issue, not a comment. See the paths above." >&2
  exit 1
fi

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
find src -name "*.cpp" -print0 | xargs -0 clang-tidy -p build

section "build"
cmake --build build --parallel

printf '\n== ALL CHECKS PASSED ==\n'
