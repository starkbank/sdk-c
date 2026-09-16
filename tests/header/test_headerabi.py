"""Negative tests for the header reader and the emitter.

A gate that cannot fail is not a gate. These cases prove the three ways the
tooling could pass while being wrong: a declaration silently not read, a macro
silently left in the cdef text, and a C type silently emitted untranslated.
"""

import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import emit
import headerabi


FAILURES = []


def check(name, condition):
    if condition:
        return
    FAILURES.append(name)
    sys.stderr.write("FAIL %s\n" % name)


def writeTemp(text):
    handle = tempfile.NamedTemporaryFile("w", suffix=".h", delete=False)
    handle.write(text)
    handle.close()
    return handle.name


def testCountGuardFires():
    """A declaration the regex cannot read must be an error, not a shrug."""
    path = writeTemp(
        "STARKBANK_API int STARKBANK_CALL starkbank_ok(void);\n"
        "STARKBANK_API int starkbank_missing_call_macro(void);\n"
    )
    try:
        headerabi.readHeader(path)
        check("count guard fires", False)
    except ValueError:
        check("count guard fires", True)
    os.unlink(path)


def testSlashSlashCommentRejected():
    path = writeTemp("// not C89\nSTARKBANK_API int STARKBANK_CALL starkbank_ok(void);\n")
    try:
        headerabi.readHeader(path)
        check("// comment rejected", False)
    except ValueError:
        check("// comment rejected", True)
    os.unlink(path)


def testCdefStripsMacros():
    header = headerabi.readHeader(os.path.join(ROOT, "include", "starkbank.h"))
    text = headerabi.cdefText(header)
    for macro in headerabi.MACROS:
        check("cdef strips %s" % macro, macro not in text)
    check("cdef drops directives", "#define" not in text and "#include" not in text)
    check("cdef drops extern C", 'extern "C"' not in text)
    check("cdef forward-declares starkcore_json", "typedef struct starkcore_json" in text)


def testUnknownTypeIsFatal():
    """An untranslated type in a binding corrupts a stack; it must never pass."""
    for table in (emit.PASCAL_TYPES, emit.CSHARP_TYPES):
        try:
            emit.translate("struct tm *", table, "x", "y")
            check("unknown type is fatal", False)
        except ValueError:
            check("unknown type is fatal", True)


def testNormalizeType():
    check("normalize collapses stars",
          emit.normalizeType("const char * *") == "const char **")
    check("normalize keeps depth",
          emit.normalizeType("starkbank_entity **") == "starkbank_entity **")


def testReservedWordsEscaped():
    check("pascal escapes out", emit.escape("out", emit.PASCAL_RESERVED, "&") == "&out")
    check("csharp escapes params", emit.escape("params", emit.CSHARP_RESERVED, "@") == "@params")
    check("plain names untouched", emit.escape("client", emit.CSHARP_RESERVED, "@") == "client")


def main():
    testCountGuardFires()
    testSlashSlashCommentRejected()
    testCdefStripsMacros()
    testUnknownTypeIsFatal()
    testNormalizeType()
    testReservedWordsEscaped()
    if FAILURES:
        sys.stderr.write("%d failed\n" % len(FAILURES))
        return 1
    sys.stdout.write("tooling: all cases passed\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
