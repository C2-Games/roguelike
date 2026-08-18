#!/usr/bin/env bash
# Runs cppcheck and (if a compile_commands.json can be found) clang-tidy over
# a C/C++ project, using the project's own .clang-tidy if present, otherwise
# dropping in this skill's bugprone/performance/readability fallback config.
#
# Usage:
#   check.sh [src-dir] [include-dir]
#
# Defaults: src-dir=src, include-dir=include (both relative to cwd).
# clang-tidy needs compile_commands.json; this script looks for one under
# ./build, ./.build/debug, ./.build/release, and cwd, in that order, and
# skips the clang-tidy step (cppcheck still runs) if none is found.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASSET_CONFIG="$SCRIPT_DIR/../assets/.clang-tidy"

src_dir="${1:-src}"
include_dir="${2:-include}"

if [[ ! -f .clang-tidy ]]; then
  echo "No .clang-tidy in $(pwd); copying skill default (bugprone-*/performance-*/readability-*)." >&2
  cp "$ASSET_CONFIG" .clang-tidy
fi

echo "== cppcheck =="
cppcheck --enable=all --std=c++17 --error-exitcode=1 \
  --suppress=missingIncludeSystem --suppress=unusedFunction \
  -I "$include_dir" "$src_dir"

compile_db=""
for candidate in build .build/debug .build/release .; do
  if [[ -f "$candidate/compile_commands.json" ]]; then
    compile_db="$candidate"
    break
  fi
done

if [[ -z "$compile_db" ]]; then
  echo "No compile_commands.json found (checked build/, .build/debug/, .build/release/, .); skipping clang-tidy." >&2
  exit 0
fi

echo "== clang-tidy (using $compile_db/compile_commands.json) =="
find "$src_dir" -name '*.cpp' -print0 | xargs -0 clang-tidy -p "$compile_db"
