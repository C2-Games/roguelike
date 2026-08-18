#!/usr/bin/env bash
# Runs the same checks as .github/workflows/ci.yml, locally.
# Usage: ./scripts/ci-local.sh
#
# Fails fast on the first check that errors. Uses build/ (not .build/debug or
# .build/release) so compile_commands.json matches what ci.yml expects.

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

section() { printf '\n== %s ==\n' "$1"; }

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
# cppcheck >= 2.19 emits under --enable=all; --error-exitcode=1 counts them as
# failures. Keep these flags in sync with .github/workflows/ci.yml.
cppcheck --enable=all --std=c++20 --error-exitcode=1 \
  --suppress=missingIncludeSystem --suppress=unusedFunction \
  --suppress=checkersReport --suppress=normalCheckLevelMaxBranches \
  -I include src/ include/

section "clang-tidy"
find src -name "*.cpp" -print0 | xargs -0 clang-tidy -p build

section "build"
cmake --build build --parallel

printf '\n== ALL CHECKS PASSED ==\n'
