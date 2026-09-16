#!/bin/sh
# The header gate. Nothing in this repo compiles yet except the header, and
# the header is the artifact with the longest blast radius, so it gets its own
# standing check before the engine exists.
#
#   1. C89 with -pedantic-errors: the dialect every FFI header translator reads.
#   2. C++98: proves the extern "C" guard, which otherwise fails at a consumer's
#      link step months later.
#   3. cffi/pycparser: proves the header is machine-readable at all.
#
# Compile-only throughout: there is no library to link against yet, and the
# gate is about the header, not about symbols.

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
CC=${CC:-cc}
CXX=${CXX:-c++}
CORE_PREFIX=${CORE_PREFIX:-$(
    for candidate in "$ROOT/../../starkinfra/core-c" /usr/local /usr; do
        if [ -f "$candidate/include/starkcore.h" ]; then echo "$candidate"; break; fi
    done
)}

if [ -z "$CORE_PREFIX" ]; then
    echo "gate: core-c not found; set CORE_PREFIX" >&2
    exit 1
fi

INCLUDES="-I$ROOT/include -I$CORE_PREFIX/include"
WARNINGS="-Werror -Wall -Wextra -Wshadow -Wpointer-arith -Wwrite-strings -Wcast-qual"

echo "gate: house rules"
python3 "$ROOT/tools/houserules.py" "$ROOT/include/starkbank.h"

echo "gate: C89  ($CC -std=c89 -pedantic-errors)"
$CC -std=c89 -pedantic-errors $WARNINGS $INCLUDES -fsyntax-only "$ROOT/tests/header/c89.c"

echo "gate: C++98 ($CXX -x c++ -std=c++98)"
$CXX -x c++ -std=c++98 -pedantic-errors $WARNINGS $INCLUDES -fsyntax-only "$ROOT/tests/header/cxx98.cpp"

# cffi is the one step here with a dependency outside the standard library,
# and installing it means reaching PyPI. A make target in a repo whose entire
# test discipline is "no network" must not do that on its own, so the step is
# opt-in and skips loudly: an offline or air-gapped build gets a sentence, not
# a pip traceback. CI sets STARKBANK_GATE_CFFI=1, so the machine-readability
# evidence still accumulates where the network is expected.
#
# The venv lives outside the repo and outside the user's site-packages: cffi is
# a gate dependency, never a build or runtime one.
VENV=${STARKBANK_GATE_VENV:-/tmp/starkbank-sdk-c-gate-venv}
CFFI_PYTHON=""
if python3 -c 'import cffi' >/dev/null 2>&1; then
    CFFI_PYTHON=python3
fi
if [ -z "$CFFI_PYTHON" ] && [ -x "$VENV/bin/python" ]; then
    CFFI_PYTHON="$VENV/bin/python"
fi
if [ -z "$CFFI_PYTHON" ] && [ "${STARKBANK_GATE_CFFI:-0}" = "1" ]; then
    echo "gate: creating $VENV"
    python3 -m venv "$VENV"
    "$VENV/bin/pip" install --quiet --disable-pip-version-check cffi
    CFFI_PYTHON="$VENV/bin/python"
fi
if [ -z "$CFFI_PYTHON" ]; then
    echo "gate: cffi step skipped (cffi not importable; set STARKBANK_GATE_CFFI=1 to create $VENV)"
fi
if [ -n "$CFFI_PYTHON" ]; then
    echo "gate: cffi"
    "$CFFI_PYTHON" "$ROOT/tools/parse_cffi.py" "$ROOT/include/starkbank.h"
fi

echo "gate: tooling negative cases"
python3 "$ROOT/tests/header/test_headerabi.py"

echo "gate: bindings regenerate byte-identically"
python3 "$ROOT/tools/emit.py" --check

echo "gate: OK"
