"""The house rules the compiler cannot enforce.

-pedantic-errors rejects C99 syntax, but it has nothing to say about a header
that is valid C89 and still wrong for this ABI: `long` compiles everywhere and
is 32 bits on Win64 and 64 bits on LP64, which silently truncates an amount on
exactly one of a caller's two platforms. Rules like that are ours, so we check
them ourselves.

Checked against the comment-stripped header, so the banner may explain a rule
using the word the rule bans.
"""

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import headerabi


# Each rule is (pattern, why it is banned). The reason is printed on a hit,
# because a rule nobody understands is a rule someone deletes.
RULES = (
    (r"\blong\b", "long is 32 bits on Win64 and 64 bits on LP64"),
    (r"\bbool\b|\b_Bool\b", "bool is not C89 and has no fixed ABI size"),
    (r"\bu?int(8|16|32|64)_t\b", "stdint types are not C89"),
    (r"\bstdint\.h\b|\bstdbool\.h\b", "neither header is C89"),
    (r"\binline\b", "inline is not C89"),
    (r"\brestrict\b", "restrict is not C89"),
    (r"\.\.\.", "a variadic entry point cannot be bound by an FFI host"),
    (r"\bstruct\s+\w+\s*\{", "a visible struct puts layout in the ABI"),
    (r"\benum\b", "an enum's underlying type is implementation-defined"),
)


def main(argv):
    if len(argv) != 2:
        sys.stderr.write("usage: houserules.py <path to starkbank.h>\n")
        return 2

    with open(argv[1], "r") as handle:
        raw = handle.read()

    # stripComments raises on a // that is not inside a /* */ comment or a
    # string, which is the only meaning of "// comment" worth checking: the
    # banner explains the rule using the token the rule bans.
    try:
        body = headerabi.stripComments(raw)
    except ValueError as reason:
        sys.stderr.write("houserules: %s\n" % reason)
        return 1
    failures = 0
    for pattern, reason in RULES:
        for match in re.finditer(pattern, body):
            line = body.count("\n", 0, match.start()) + 1
            sys.stderr.write("houserules: %s:%d: %r - %s\n"
                             % (argv[1], line, match.group(0), reason))
            failures += 1

    if failures:
        return 1
    sys.stdout.write("houserules: %d rules, no violations\n" % len(RULES))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
