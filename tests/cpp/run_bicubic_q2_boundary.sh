#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="${TMPDIR:-/tmp}/pdfxtmd_bicubic_q2_boundary_${UID:-0}"
mkdir -p "$BUILD"
"${CXX:-g++}" -std=c++17 -O2 -Wall -Wextra -I"$ROOT/include" \
    "$ROOT/tests/cpp/bicubic_q2_boundary.cpp" \
    "$ROOT/src/Common/AllFlavorsShape.cpp" \
    -o "$BUILD/bicubic_q2_boundary"
"$BUILD/bicubic_q2_boundary"
