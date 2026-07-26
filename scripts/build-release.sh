#!/usr/bin/env bash
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"
cmake -B .build/release -S .
cmake --build .build/release
