"""Reader for include/starkbank.h.

The header is the only authoring surface for the ABI, so every downstream tool
-- the cffi parse gate and the Delphi/.NET emitters -- reads the header itself
rather than a JSON description someone has to keep in step with it. Python 3
stdlib only; no cffi import here, so the emitters run on a machine with no venv.

The parse is deliberately shallow and strict: it understands exactly the shapes
the header is allowed to contain, and raises on anything else. A header
construct this file cannot read is a header construct an FFI host cannot read
either, which is the whole point of the gate.
"""

import re


MACROS = ("STARKBANK_API", "STARKBANK_CALL", "STARKCORE_API", "STARKCORE_CALL")

# One declaration per statement, so the extractor never has to balance braces.
DECL_RE = re.compile(
    r"^STARKBANK_API\s+(?P<ret>.+?)\s*STARKBANK_CALL\s+"
    r"(?P<name>starkbank_[a-z0-9_]+)\s*\((?P<args>.*?)\)\s*;",
    re.S | re.M,
)

DEFINE_RE = re.compile(
    r"^#define\s+(?P<name>STARKBANK_[A-Z0-9_]+)\s+(?P<value>\(?-?[0-9]+\)?|\"[^\"]*\")\s*$",
    re.M,
)


def stripComments(text):
    """Remove C comments without touching string literals that contain '/*'."""
    out = []
    index = 0
    length = len(text)
    while index < length:
        char = text[index]
        if char == '"':
            end = index + 1
            while end < length and text[end] != '"':
                end += 2 if text[end] == "\\" else 1
            out.append(text[index:end + 1])
            index = end + 1
            continue
        if text.startswith("/*", index):
            end = text.find("*/", index + 2)
            if end < 0:
                raise ValueError("unterminated comment in header")
            out.append(" ")
            index = end + 2
            continue
        if text.startswith("//", index):
            raise ValueError("// comment in a C89 header at offset %d" % index)
        out.append(char)
        index += 1
    return "".join(out)


def splitArgs(text):
    """Split a parameter list on top-level commas; function-pointer params nest."""
    args = []
    depth = 0
    current = ""
    for char in text:
        if char == "(":
            depth += 1
        if char == ")":
            depth -= 1
        if char == "," and depth == 0:
            args.append(current.strip())
            current = ""
            continue
        current += char
    if current.strip():
        args.append(current.strip())
    return args


def readHeader(path):
    with open(path, "r") as handle:
        raw = handle.read()
    body = stripComments(raw)
    body = re.sub(r"[ \t]+", " ", body)

    functions = []
    for match in DECL_RE.finditer(body):
        args = splitArgs(match.group("args"))
        if args == ["void"]:
            args = []
        functions.append({
            "name": match.group("name"),
            "ret": match.group("ret").strip(),
            "args": args,
        })

    # Read the constants out of the comment-stripped text: nearly every define
    # in the header carries a trailing comment, and matching against the raw
    # text silently skipped exactly those.
    # A declaration the regex cannot read would shrink every downstream gate in
    # silence, which is the one failure mode a gate must not have.
    expected = len([
        line for line in body.splitlines() if line.lstrip().startswith("STARKBANK_API")
    ])
    if len(functions) != expected:
        raise ValueError(
            "read %d declarations but the header has %d STARKBANK_API lines"
            % (len(functions), expected))

    constants = {}
    for match in DEFINE_RE.finditer(re.sub(r"[ \t]+", " ", body)):
        constants[match.group("name")] = match.group("value")

    return {"path": path, "functions": functions, "constants": constants, "text": raw}


def cdefText(header):
    """The header reduced to what ffi.cdef() accepts.

    cffi parses declarations, not a preprocessor: directives, the extern "C"
    guard and the two calling-convention macros all go, and the starkcore
    handles the transport seam names are forward-declared instead of pulled in
    from starkcore.h. Nothing else is rewritten -- if a declaration needs
    special handling to parse, the header is wrong, not this function.
    """
    body = stripComments(header["text"])
    lines = []
    for line in body.splitlines():
        stripped = line.strip()
        if stripped.startswith("#"):
            continue
        if stripped.startswith('extern "C"') or stripped in ("{", "}"):
            continue
        lines.append(line)
    body = "\n".join(lines)
    for macro in MACROS:
        body = body.replace(macro, " ")
    forward = "\n".join(
        "typedef struct %s %s;" % (name, name)
        for name in ("starkcore_json", "starkcore_headers", "starkcore_response", "starkcore_errors")
    )
    return forward + "\n" + body
