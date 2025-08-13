#!/usr/bin/env bash
set -euo pipefail
if [ -z "${ONNXRUNTIME_ROOT:-}" ]; then echo "Set ONNXRUNTIME_ROOT to your onnxruntime folder"; exit 1; fi
cd "$(dirname "$0")"
cmake -B build -G Xcode -DONNXRUNTIME_ROOT="$ONNXRUNTIME_ROOT"
cmake --build build --config Release
echo "Built. Find AmpProfiler.vst3 in plugin/build/Release/"
