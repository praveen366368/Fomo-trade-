#!/usr/bin/env bash
# Run a paper-trading session on live market data.
#   scripts/paper.sh 60          -> 60 minutes, results in data/run-<time>/
# Stop early: touch data/run-<time>/STOP   Block new buys: touch .../KILL
set -euo pipefail
cd "$(dirname "$0")/.."
minutes="${1:-60}"
dir="data/run-$(date -u +%Y%m%d-%H%M%S)"
mkdir -p "$dir"
[ -x engine/build/fomo-engine ] || scripts/check.sh
echo "paper run: $minutes min -> $dir (log: $dir/engine.log)"
engine/build/fomo-engine --config config/fomo.conf --minutes "$minutes" --data "$dir" 2>&1 | tee "$dir/engine.log"
