#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"
cmake -B .build/debug -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build .build/debug