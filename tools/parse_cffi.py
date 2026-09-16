"""The FFI parse gate: every declaration in include/starkbank.h must parse.

Design risk 2 is that a hand-maintained header does not parse in the tools an
FFI host brings. cffi's parser is pycparser, which is the same parser most
Python, ctypes and header-translation tooling ends up using, so a clean
ffi.cdef() is the cheapest standing proof that the header is machine-readable.

Run with a python that has cffi (tools/gate.sh builds a throwaway venv).
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import headerabi


def main(argv):
    if len(argv) != 2:
        sys.stderr.write("usage: parse_cffi.py <path to starkbank.h>\n")
        return 2

    import cffi

    header = headerabi.readHeader(argv[1])
    ffi = cffi.FFI()
    ffi.cdef(headerabi.cdefText(header))

    # A stripper that silently dropped declarations would make the gate pass by
    # parsing less, so every name the header declares is resolved back out of
    # the parsed set, not merely counted.
    declared = sorted(item["name"] for item in header["functions"])
    if not declared:
        sys.stderr.write("no STARKBANK_API declarations found; the header or the reader is wrong\n")
        return 1

    # ffi.typeof resolves types, not functions, and there is no public way to
    # ask an uncompiled FFI what functions it parsed. The parser's declaration
    # map is the only answer, and a gate is the right place to reach for one.
    parsed = set(
        key.split(" ", 1)[1] for key in ffi._parser._declarations
        if key.startswith("function ")
    )
    missing = [name for name in declared if name not in parsed]
    if missing:
        sys.stderr.write("cffi did not parse %d declarations: %s\n"
                         % (len(missing), ", ".join(missing[:5])))
        return 1

    sys.stdout.write("cffi: %d declarations parsed, %d constants read\n"
                     % (len(declared), len(header["constants"])))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
