"""Emits the Delphi unit and the C# P/Invoke class from include/starkbank.h.

Bindings are shipped, not merely enabled: a team with no C programmer must be
able to adopt this on day one, and exposing a narrow ABI so a host *can* write
its own wrapper and then not writing one wastes the reason the ABI is narrow.

Nothing this emits is part of the ABI, and nothing it emits ever ships inside
the library. That boundary is the whole design: the C is authored by hand,
permanently, and a generator only ever produces artifacts outside it.

Output is byte-deterministic - LF, no timestamps, no paths, and no generator
version string inside any emitted file, so a generator bump never rewrites the
tree and buries the real diff. The version lives in bindings/.genstamp alone.
CI runs this with --check and fails on any difference, so the emitted files
cannot be hand-edited.

Python 3 stdlib only. `make`, `make test` and `make install` never call it: a
vendor building from a source tarball may have no Python at all.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import headerabi
import drift


GENSTAMP = "emit.py 3"

# Filled from the public header rather than duplicated here: the emitter must
# not be a second place that believes it knows what type 8 is.
FIELD_TYPE_NAMES = {}

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "starkbank.h")

# Every C spelling the header is allowed to use, mapped to each host language.
# A spelling that is not in here is a spelling the header should not contain,
# so an unknown type is a hard failure rather than a passthrough: silently
# emitting an untranslated type is how a binding corrupts a stack.
PASCAL_TYPES = {
    "void": "",
    "int": "Integer",
    "double": "Double",
    "size_t": "NativeUInt",
    "const char *": "PAnsiChar",
    "const char **": "PPAnsiChar",
    "char **": "PPAnsiChar",
    "const unsigned char *": "PByte",
    "unsigned char **": "PPByte",
    "size_t *": "PNativeUInt",
    "int *": "PInteger",
    "double *": "PDouble",
    "void *": "Pointer",
}

CSHARP_TYPES = {
    "void": "void",
    "int": "int",
    "double": "double",
    "size_t": "UIntPtr",
    "const char *": "IntPtr",
    "const char **": "out IntPtr",
    "char **": "out IntPtr",
    "const unsigned char *": "IntPtr",
    "unsigned char **": "out IntPtr",
    "size_t *": "out UIntPtr",
    "int *": "out int",
    "double *": "out double",
    "void *": "IntPtr",
}

# Opaque handles cross as a machine word and nothing more: no layout, no type
# name. This first cut deliberately uses a bare pointer rather than a
# SafeHandle, so the mapping is exactly the ABI and nothing is hidden.
# Reserved in the host language and therefore never usable as a parameter
# name. The header's own names are C-idiomatic and stay as they are; the
# binding escapes them, because renaming would make the two surfaces diverge.
PASCAL_RESERVED = frozenset((
    "and", "array", "as", "begin", "case", "const", "div", "do", "downto",
    "else", "end", "except", "file", "for", "function", "goto", "if", "in",
    "inherited", "label", "mod", "nil", "not", "object", "of", "or", "out",
    "packed", "procedure", "program", "record", "repeat", "set", "shl", "shr",
    "string", "then", "to", "type", "unit", "until", "uses", "var", "while",
    "with", "xor",
))

CSHARP_RESERVED = frozenset((
    "base", "bool", "byte", "case", "char", "class", "const", "default",
    "delegate", "do", "double", "else", "event", "extern", "false", "fixed",
    "float", "for", "in", "int", "interface", "internal", "is", "lock", "long",
    "namespace", "new", "null", "object", "operator", "out", "override",
    "params", "private", "public", "readonly", "ref", "return", "sealed",
    "short", "sizeof", "static", "string", "struct", "switch", "this", "true",
    "typeof", "uint", "ulong", "unsafe", "ushort", "using", "virtual", "void",
    "while",
))

HANDLES = (
    "starkbank_user", "starkbank_client", "starkbank_entity", "starkbank_list",
    "starkbank_iter", "starkbank_errors", "starkbank_headers", "starkbank_response",
    "starkcore_json",
)


def normalizeType(text):
    """One canonical spelling per C type, so the tables need only one key each.

    "const char*", "const char *" and "const char * *" all reach the tables as
    "const char *" / "const char **": pointer depth is what matters to a
    binding, and whitespace around a star is not.
    """
    stars = text.count("*")
    words = text.replace("*", " ").split()
    if not stars:
        return " ".join(words)
    return " ".join(words) + " " + "*" * stars


def splitDeclarator(argument):
    """A C parameter into (canonical type, name)."""
    text = " ".join(argument.split())
    if text.startswith("starkbank_transport_fn"):
        return ("starkbank_transport_fn", "transport")
    parts = text.rsplit(" ", 1)
    if len(parts) == 1:
        return (normalizeType(text), "value")
    typeText, name = parts
    while name.startswith("*"):
        typeText += "*"
        name = name[1:]
    return (normalizeType(typeText), name)


def escape(name, reserved, marker):
    if name in reserved:
        return marker + name
    return name


def opaqueBase(typeText):
    """The handle name in a pointer-to-handle type, or None."""
    words = typeText.replace("*", " ").split()
    for word in words:
        if word in HANDLES:
            return word
    return None


def translate(typeText, table, pointerText, handleText):
    if typeText in table:
        return table[typeText]
    if opaqueBase(typeText) is not None:
        stars = typeText.count("*")
        if stars >= 2:
            return pointerText
        return handleText
    raise ValueError("no binding for C type %r" % typeText)


def pascalUnit(header):
    lines = [
        "unit starkbank;",
        "",
        "{ Delphi binding for libstarkbank. Emitted from include/starkbank.h by",
        "  tools/emit.py; edit the header, never this file.",
        "",
        "  Handles are opaque pointers and nothing about their layout crosses the",
        "  boundary. Every string returned is borrowed from the entity or list it",
        "  came from and must not be freed; every buffer the library allocates is",
        "  released with starkbank_free and never with FreeMem. }",
        "",
        "interface",
        "",
        "const",
        "{$IFDEF MSWINDOWS}",
        "  StarkbankLib = 'starkbank.dll';",
        "{$ENDIF}",
        "{$IFDEF LINUX}",
        "  StarkbankLib = 'libstarkbank.so.1';",
        "{$ENDIF}",
        "{$IFDEF MACOS}",
        "  StarkbankLib = 'libstarkbank.dylib';",
        "{$ENDIF}",
        "",
    ]

    for name in sorted(header["constants"]):
        value = header["constants"][name]
        if value.startswith("\""):
            value = "'" + value[1:-1] + "'"
        lines.append("  %s = %s;" % (name, value.strip("()")))

    lines.extend([
        "",
        "type",
        "  { One machine word. Declared distinctly so a Transfer handle cannot be",
        "    passed where an Invoice entity is wanted by accident. }",
    ])
    for handle in sorted(HANDLES):
        lines.append("  P%s = type Pointer;" % handle)
    for handle in sorted(HANDLES):
        lines.append("  PP%s = ^P%s;" % (handle, handle))
    lines.extend([
        "",
        "  TStarkbankTransport = function(context: Pointer; method: Integer;",
        "    url: PAnsiChar; headers: Pstarkbank_headers; body: PAnsiChar;",
        "    bodyLen: NativeUInt; timeoutSeconds: Integer;",
        "    out response: Pstarkbank_response): Integer; cdecl;",
        "",
    ])

    for function in sorted(header["functions"], key=lambda item: item["name"]):
        lines.extend(pascalDeclaration(function))

    lines.extend(["", "implementation", "", "end."])
    return "\n".join(lines) + "\n"


def pascalDeclaration(function):
    parameters = []
    for argument in function["args"]:
        typeText, name = splitDeclarator(argument)
        if typeText == "starkbank_transport_fn":
            parameters.append("%s: TStarkbankTransport" % name)
            continue
        base = opaqueBase(typeText)
        parameters.append("%s: %s" % (
            escape(name, PASCAL_RESERVED, "&"),
            translate(typeText, PASCAL_TYPES, "PP" + (base or ""), "P" + (base or ""))))

    returnType = normalizeType(function["ret"])
    returnBase = opaqueBase(returnType)
    returnText = translate(returnType, PASCAL_TYPES, "PP" + (returnBase or ""),
                           "P" + (returnBase or ""))
    keyword = "procedure" if returnText == "" else "function"
    argumentText = "(%s)" % "; ".join(parameters) if parameters else ""
    signature = "%s %s%s" % (keyword, function["name"], argumentText)
    if returnText != "":
        signature += ": %s" % returnText
    # Delphi defaults to stdcall for external declarations, so cdecl is spelled
    # on every one of them: getting this wrong corrupts the stack on Win32 and
    # is invisible everywhere else.
    return ["%s; cdecl; external StarkbankLib name '%s';" % (signature, function["name"]), ""]


def csharpClass(header):
    lines = [
        "// C# binding for libstarkbank. Emitted from include/starkbank.h by",
        "// tools/emit.py; edit the header, never this file.",
        "//",
        "// Handles are IntPtr: this first cut maps the ABI exactly and hides",
        "// nothing. Strings come back as a borrowed pointer into the entity that",
        "// owns them, so Marshal.PtrToStringAnsi copies before the entity is freed;",
        "// buffers the library allocated are released with StarkbankFree, never by",
        "// the .NET runtime.",
        "",
        "using System;",
        "using System.Runtime.InteropServices;",
        "",
        "namespace StarkBank",
        "{",
        "    public static partial class Native",
        "    {",
        "        private const string Library = \"starkbank\";",
        "",
    ]

    for name in sorted(header["constants"]):
        value = header["constants"][name]
        if value.startswith("\""):
            lines.append("        public const string %s = %s;" % (csharpName(name), value))
            continue
        lines.append("        public const int %s = %s;" % (csharpName(name), value.strip("()")))

    lines.extend([
        "",
        "        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]",
        "        public delegate int Transport(IntPtr context, int method,",
        "            [MarshalAs(UnmanagedType.LPStr)] string url, IntPtr headers,",
        "            IntPtr body, UIntPtr bodyLen, int timeoutSeconds, out IntPtr response);",
        "",
    ])

    for function in sorted(header["functions"], key=lambda item: item["name"]):
        lines.extend(csharpDeclaration(function))

    lines.extend(["    }", "}"])
    return "\n".join(lines) + "\n"


def csharpName(name):
    return "".join(part.capitalize() for part in name.split("_"))


def csharpDeclaration(function):
    parameters = []
    for argument in function["args"]:
        typeText, name = splitDeclarator(argument)
        if typeText == "starkbank_transport_fn":
            parameters.append("Transport %s" % name)
            continue
        name = escape(name, CSHARP_RESERVED, "@")
        if typeText == "const char *":
            parameters.append("[MarshalAs(UnmanagedType.LPStr)] string %s" % name)
            continue
        parameters.append("%s %s" % (translate(typeText, CSHARP_TYPES, "out IntPtr", "IntPtr"), name))

    returnType = normalizeType(function["ret"])
    returnText = translate(returnType, CSHARP_TYPES, "IntPtr", "IntPtr")
    if returnType == "const char *":
        # Not a marshalled string: the pointer is borrowed and the runtime must
        # not try to free it, which is exactly what a string return would do.
        returnText = "IntPtr"
    return [
        "        [DllImport(Library, EntryPoint = \"%s\", CallingConvention = CallingConvention.Cdecl)]"
        % function["name"],
        "        public static extern %s %s(%s);" % (returnText, csharpName(function["name"]),
                                                     ", ".join(parameters)),
        "",
    ]


# ------------------------------------------------------------------ samples
# A docs sample is a program here, not a string in app-docs, and `make samples`
# compiles every one of them with the library's own warning set. No other
# language on that site can claim that.
#
# Only the mechanical verbs are emitted. The judgement methods get hand-written
# samples under samples/handwritten/, for the same reason they get hand-written
# implementations.

SAMPLE_VALUES = {
    # PaymentPreview is the one resource where id is a create parameter: it is
    # the payment code being previewed, not an object's identifier.
    "id": "\"34191.09008 63571.277308 71444.640008 5 81960000000062\"",
    "url": "\"https://webhook.site/60e9c18e-4b5c-4369-bda1-ab5fcd8e1b29\"",
    "taxId": "\"012.345.678-90\"",
    "name": "\"Arya Stark\"",
    "bankCode": "\"20018183\"",
    "branchCode": "\"1357-9\"",
    "accountNumber": "\"876543-2\"",
    "accountType": "\"checking\"",
    "externalId": "\"my-internal-id-123456\"",
    "receiverId": "\"5706627130851328\"",
    "description": "\"Transaction to dear provider\"",
    "key": "\"allowedTaxIds\"",
}

SAMPLE_FILTERS = {
    "status": "\"paid\"",
}

# A LIST_STRING whose members are a vocabulary rather than free text. A
# Webhook sample that subscribes to "war" would be a sample nobody can paste.
SAMPLE_LIST_VALUES = {
    "subscriptions": "\"invoice\"",
}

PROLOGUE = """#include <stdio.h>
#include <stdlib.h>

#include "starkbank.h"

/*<*/
static starkbank_client *connect(void)
{
    starkbank_user *project = NULL;
    starkbank_client *client = NULL;

    if (starkbank_project_new("5656565656565656", STARKBANK_ENVIRONMENT_SANDBOX,
                              getenv("STARK_PRIVATE_KEY"), &project) != STARKBANK_OK) {
        return NULL;
    }
    if (starkbank_client_new(project, &client) != STARKBANK_OK) {
        return NULL;
    }
    starkbank_client_set_curl_transport(client);
    return client;
}

static int report(int status, starkbank_errors *errors)
{
    int index;

    fprintf(stderr, "%s\\n", starkbank_strerror(status));
    for (index = 0; index < starkbank_errors_count(errors); index++) {
        fprintf(stderr, "  %s: %s\\n", starkbank_errors_code_at(errors, index),
                starkbank_errors_message_at(errors, index));
    }
    starkbank_errors_free(errors);
    return 1;
}
/*>*/
"""


def sampleName(ident, verb):
    return "%s-%s.c" % (ident.replace("_", "-"), verb)


def fieldTypeName(value):
    """STARKBANK_FIELD_* by number, read from the header once and never copied."""
    global FIELD_TYPE_NAMES
    if not FIELD_TYPE_NAMES:
        with open(HEADER) as handle:
            FIELD_TYPE_NAMES = drift.fieldTypeNames(handle.read())
    return FIELD_TYPE_NAMES[value]


def sampleValue(field):
    """One example value per field, keyed by name where the name means something."""
    key = field["key"]
    typeName = fieldTypeName(field["type"])
    if key in SAMPLE_VALUES:
        return ("set_string", SAMPLE_VALUES[key])
    if typeName == "AMOUNT":
        return ("set_amount", "400000")
    if typeName == "RATE":
        return ("set_number", "2.5")
    if typeName == "NUMBER":
        return ("set_number", "5")
    if typeName == "SECONDS":
        return ("set_seconds", "123456789")
    if typeName == "BOOL":
        return ("set_bool", "1")
    if typeName in ("DATE", "DATE_OR_DATETIME"):
        return ("set_date", "2026, 10, 28")
    if typeName == "DATETIME":
        return ("set_datetime", "2026, 10, 28, 17, 59, 26")
    if typeName == "LIST_STRING":
        return ("append_string", SAMPLE_LIST_VALUES.get(key, "\"war\""))
    if typeName == "STRING":
        return ("set_string", "\"example\"")
    return (None, None)


def requiredSetters(ident, table):
    lines = []
    for field in table["fields"]:
        if not field["flags"] & drift.FLAG_REQUIRED:
            continue
        setter, value = sampleValue(field)
        if setter is None:
            continue
        lines.append("    starkbank_entity_%s(%s, \"%s\", %s);"
                     % (setter, ident, field["key"], value))
    return lines


def patchSetter(ident, table):
    for field in table["fields"]:
        if not field["flags"] & drift.FLAG_PATCH:
            continue
        setter, value = sampleValue(field)
        if setter is None:
            continue
        return "    starkbank_entity_%s(patch, \"%s\", %s);" % (setter, field["key"], value)
    return None


def filterSetter(table):
    for key in table["queryKeys"]:
        if key in SAMPLE_FILTERS:
            return "    starkbank_entity_set_string(params, \"%s\", %s);" % (key, SAMPLE_FILTERS[key])
    return None


def sampleBody(ident, table, verb, shape):
    """The body of one sample, by verb shape. One function, one job: each arm
    is the smallest complete program that calls that verb and frees everything."""
    call = "starkbank_%s_%s" % (ident, verb)
    if shape == "POST_MULTI":
        setters = "\n".join(requiredSetters(ident, table))
        return ("""    starkbank_list *batch = NULL;
    starkbank_list *created = NULL;
    starkbank_entity *%(ident)s = NULL;
    starkbank_errors *errors = NULL;
    const char *id = NULL;
    int status;

    starkbank_%(ident)s_new(&%(ident)s);
%(setters)s
    starkbank_list_new(&batch);
    starkbank_list_append(batch, %(ident)s);        /* the list owns it from here */

    status = %(call)s(client, batch, &created, &errors);
    starkbank_list_free(batch);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    starkbank_entity_string(starkbank_list_at(created, 0), "id", &id);
    printf("created %%s\\n", id);
    starkbank_list_free(created);
""" % {"ident": ident, "setters": setters, "call": call})
    if shape in ("POST_SINGLE", "POST_SINGLE_SUB"):
        setters = "\n".join(requiredSetters(ident, table))
        return ("""    starkbank_entity *%(ident)s = NULL;
    starkbank_entity *created = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_%(ident)s_new(&%(ident)s);
%(setters)s
    status = %(call)s(client, %(ident)s, &created, &errors);
    starkbank_entity_free(%(ident)s);       /* post_single borrows it, unlike a list */
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("created %%s\\n", starkbank_entity_id(created));
    starkbank_entity_free(created);
""" % {"ident": ident, "setters": setters, "call": call})
    if shape == "GET_ID":
        return ("""    starkbank_entity *%(ident)s = NULL;
    starkbank_errors *errors = NULL;
    int status;

    status = %(call)s(client, "5656565656565656", &%(ident)s, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("%%s\\n", starkbank_entity_id(%(ident)s));
    starkbank_entity_free(%(ident)s);
""" % {"ident": ident, "call": call})
    if shape == "GET_FIRST":
        return ("""    starkbank_entity *%(ident)s = NULL;
    starkbank_errors *errors = NULL;
    double amount = 0;
    int status;

    status = %(call)s(client, &%(ident)s, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    starkbank_entity_amount(%(ident)s, "amount", &amount);
    printf("%%.0f\\n", amount);
    starkbank_entity_free(%(ident)s);
""" % {"ident": ident, "call": call})
    if shape == "QUERY":
        setter = filterSetter(table)
        return ("""    starkbank_entity *params = NULL;
    starkbank_iter *iter = NULL;
    const starkbank_entity *found = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_%(ident)s_params_new(&params);
%(setter)s
    status = %(call)s(client, params, 100, &iter);
    if (status != STARKBANK_OK) {
        starkbank_entity_free(params);
        starkbank_client_free(client);
        return report(status, NULL);
    }
    /* A network error surfaces here and not at the call above, exactly as in
       starkcore_stream: the first page is fetched by the first next(). */
    while ((status = starkbank_iter_next(iter, &found, &errors)) == STARKBANK_OK
           && found != NULL) {
        printf("%%s\\n", starkbank_entity_id(found));
    }
    starkbank_iter_free(iter);
    starkbank_entity_free(params);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
""" % {"ident": ident, "call": call, "setter": setter if setter else "    /* no filters */"})
    if shape == "PAGE":
        return ("""    starkbank_entity *params = NULL;
    starkbank_list *page = NULL;
    starkbank_errors *errors = NULL;
    char *cursor = NULL;
    int status;
    int index;

    starkbank_%(ident)s_params_new(&params);
    starkbank_entity_set_number(params, "limit", 10);

    status = %(call)s(client, params, &page, &cursor, &errors);
    starkbank_entity_free(params);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    for (index = 0; index < starkbank_list_count(page); index++) {
        printf("%%s\\n", starkbank_entity_id(starkbank_list_at(page, index)));
    }
    printf("cursor %%s\\n", cursor != NULL ? cursor : "");
    starkbank_free(cursor);
    starkbank_list_free(page);
""" % {"ident": ident, "call": call})
    if shape in ("PATCH_ID", "PATCH_ID_ECHO"):
        setter = patchSetter(ident, table)
        if setter is None:
            return None
        return ("""    starkbank_entity *patch = NULL;
    starkbank_entity *%(ident)s = NULL;
    starkbank_errors *errors = NULL;
    int status;

    starkbank_%(ident)s_params_new(&patch);
%(setter)s

    status = %(call)s(client, "5656565656565656", patch, &%(ident)s, &errors);
    starkbank_entity_free(patch);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("%%s\\n", starkbank_entity_id(%(ident)s));
    starkbank_entity_free(%(ident)s);
""" % {"ident": ident, "call": call, "setter": setter})
    if shape == "DELETE_ID":
        return ("""    starkbank_entity *%(ident)s = NULL;
    starkbank_errors *errors = NULL;
    int status;

    status = %(call)s(client, "5656565656565656", &%(ident)s, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("deleted %%s\\n", starkbank_entity_id(%(ident)s));
    starkbank_entity_free(%(ident)s);
""" % {"ident": ident, "call": call})
    if shape in ("CONTENT", "CONTENT_INT"):
        argument = ", 7" if shape == "CONTENT_INT" else ""
        return ("""    unsigned char *content = NULL;
    size_t length = 0;
    starkbank_errors *errors = NULL;
    FILE *file;
    int status;

    status = %(call)s(client, "5656565656565656"%(argument)s, &content, &length, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    file = fopen("%(ident)s-%(verb)s.bin", "wb");
    if (file != NULL) {
        fwrite(content, 1, length, file);
        fclose(file);
    }
    printf("%%lu bytes\\n", (unsigned long)length);
    starkbank_free(content);
""" % {"ident": ident, "verb": verb, "call": call, "argument": argument})
    if shape == "CONTENT_QUERY":
        return ("""    unsigned char *content = NULL;
    size_t length = 0;
    starkbank_errors *errors = NULL;
    FILE *file;
    int status;

    status = %(call)s(client, "5656565656565656", "booklet", "customerAddress", &content, &length, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    file = fopen("%(ident)s-%(verb)s.bin", "wb");
    if (file != NULL) {
        fwrite(content, 1, length, file);
        fclose(file);
    }
    printf("%%lu bytes\\n", (unsigned long)length);
    starkbank_free(content);
""" % {"ident": ident, "verb": verb, "call": call})
    if shape == "SUB_RESOURCE":
        return ("""    starkbank_entity *%(verb)s = NULL;
    starkbank_errors *errors = NULL;
    int status;

    status = %(call)s(client, "5656565656565656", &%(verb)s, &errors);
    if (status != STARKBANK_OK) {
        starkbank_client_free(client);
        return report(status, errors);
    }
    printf("%%s\\n", starkbank_entity_resource(%(verb)s));
    starkbank_entity_free(%(verb)s);
""" % {"verb": verb, "call": call})
    return None


def sampleSource(ident, table, verb, shape, example):
    """One sample program. `example` is the authored example block a resource
    will carry in step 6; until then the values are type-driven."""
    route = drift.verbEndpoint(shape, table["endpoint"], verb)
    if route is None:
        return None
    body = sampleBody(ident, table, verb, shape)
    if body is None:
        return None
    # Declarations first, as everything else in this repo: each arm is written
    # with its declarations, a blank line, then its statements.
    declarations, statements = body.split("\n\n", 1)
    return ("/*\n * %s %s\n *\n"
            " * Emitted from the %s table by tools/emit.py. Edit the table, never this\n"
            " * file: make check-emit regenerates it and fails on any difference.\n */\n"
            "%s\nint main(void)\n{\n    starkbank_client *client = NULL;\n%s\n\n"
            "    client = connect();\n    if (client == NULL) {\n        return 1;\n    }\n\n%s\n"
            "    starkbank_client_free(client);\n    return 0;\n}\n"
            % (route[0], route[1], table["name"], PROLOGUE, declarations, statements))


# --------------------------------------------------------------- writing out

# Three places, and nowhere else. An emitter that can write anywhere is an
# emitter that eventually writes into the ABI, and the ABI is hand-authored
# permanently.
OWNED = ("bindings", "samples", os.path.join("include", "starkbank_fields.h"))


def owned(path):
    relative = os.path.relpath(os.path.abspath(path), ROOT)
    return any(relative == entry or relative.startswith(entry + os.sep) for entry in OWNED)


def writeOrCheck(path, content, check):
    if not owned(path):
        raise ValueError("emit refuses to write outside %s: %s" % (", ".join(OWNED), path))
    existing = None
    if os.path.exists(path):
        with open(path, "r") as handle:
            existing = handle.read()
    if check:
        if existing != content:
            sys.stderr.write("emit: %s is out of date; run tools/emit.py\n" % path)
            return 1
        return 0
    if existing == content:
        return 0
    directory = os.path.dirname(path)
    if directory and not os.path.isdir(directory):
        os.makedirs(directory)
    with open(path, "w", newline="\n") as handle:
        handle.write(content)
    return 0


def sampleOutputs():
    """Every mechanical verb, from the tables and the verb macros themselves.

    The shapes come from the same reader the drift checker uses, so a verb that
    exists has a sample and a verb that does not cannot have one.
    """
    tables = drift.readTables()
    verbs = drift.readVerbs(ROOT)

    outputs = []
    for name in sorted(verbs):
        if name not in tables:
            continue
        ident = verbs[name]["ident"]
        for verb, shape in sorted(verbs[name]["verbs"].items()):
            source = sampleSource(ident, tables[name], verb, shape, None)
            if source is None:
                continue
            outputs.append((os.path.join(ROOT, "samples", sampleName(ident, verb)), source))
    return outputs


def main(argv):
    check = "--check" in argv[1:]
    samples = "--samples" in argv[1:] or check
    header = headerabi.readHeader(HEADER)

    outputs = [
        (os.path.join(ROOT, "bindings", "starkbank.pas"), pascalUnit(header)),
        (os.path.join(ROOT, "bindings", "Starkbank.cs"), csharpClass(header)),
        (os.path.join(ROOT, "bindings", ".genstamp"), GENSTAMP + "\n"),
    ]
    if samples:
        outputs.extend(sampleOutputs())

    status = 0
    for path, content in outputs:
        status |= writeOrCheck(path, content, check)
    if status == 0 and not check:
        sys.stdout.write("emit: %d declarations into %d files\n"
                         % (len(header["functions"]), len(outputs)))
    return status


if __name__ == "__main__":
    sys.exit(main(sys.argv))
