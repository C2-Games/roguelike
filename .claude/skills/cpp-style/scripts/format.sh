#!/usr/bin/env bash
# Formats (or checks) C/C++ files with clang-format, using the project's own
# .clang-format if present, otherwise dropping in this skill's Google+Allman
# fallback config.
#
# Usage:
#   format.sh [--check] [path...]
#
# With no paths, searches src/ and include/ under the current directory for
# .cpp/.h/.hpp files (falls back to the current directory if neither exists).
# --check runs a dry-run and fails (exit 1) on any file that would change,
# without modifying anything. Default mode formats files in place.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASSET_CONFIG="$SCRIPT_DIR/../assets/.clang-format"

check_mode=false
paths=()

for arg in "$@"; do
  if [[ "$arg" == "--check" ]]; then
    check_mode=true
  else
    paths+=("$arg")
  fi
done

if [[ ! -f .clang-format ]]; then
  echo "No .clang-format in $(pwd); copying skill default (Google + Allman braces)." >&2
  cp "$ASSET_CONFIG" .clang-format
fi

if [[ ${#paths[@]} -eq 0 ]]; then
  if [[ -d src || -d include ]]; then
    paths=(src include)
  else
    paths=(.)
  fi
fi

files=()
while IFS= read -r -d '' f; do
  files+=("$f")
done < <(find "${paths[@]}" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)

if [[ ${#files[@]} -eq 0 ]]; then
  echo "No .cpp/.h/.hpp files found under: ${paths[*]}" >&2
  exit 0
fi

if $check_mode; then
  clang-format --dry-run --Werror -style=file "${files[@]}"
else
  clang-format -i -style=file "${files[@]}"
fi
