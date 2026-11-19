#!/usr/bin/env bash
# Full gate: build (strict warnings = typecheck), tests, lint. Must pass before any push.
set -euo pipefail
cd "$(dirname "$0")/.."

echo "== OCaml risk daemon: build + tests"
(cd risk && dune build 2>&1 && dune test 2>&1)

echo "== C++ engine: build (-Wall -Wextra -Wpedantic -Wconversion -Werror)"
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build engine/build -j"$(nproc)"

echo "== C++ engine: tests"
engine/build/fomo-tests

echo "== C++ engine: lint (clang-tidy)"
if command -v clang-tidy >/dev/null; then
  out=$(clang-tidy -p engine/build --quiet \
    --checks='-*,bugprone-*,performance-*,clang-analyzer-*,-bugprone-easily-swappable-parameters,-bugprone-narrowing-conversions,-performance-avoid-endl,-bugprone-exception-escape' \
    engine/src/core/*.cpp engine/src/agents/*.cpp engine/src/app/*.cpp engine/src/main.cpp 2>/dev/null | grep -E "warning:|error:" || true)
  if [ -n "$out" ]; then echo "$out"; echo "lint failed"; exit 1; fi
  echo "lint clean"
else
  echo "clang-tidy not installed - skipped"
fi

echo "== config sanity: roster loads"
engine/build/fomo-engine --roster --quiet --data "$(mktemp -d)" | head -1
echo "ALL CHECKS PASSED"
