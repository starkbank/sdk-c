"""Compares the field tables with everything that can make them wrong.

The tables are derived from sdk-python and diffed against sdk-python and the
docs JSON in CI. They are never a source of truth, and this script is the only
thing standing between "derived" and "was derived once, in 2026".

It reads and never writes. It never imports sdk-python - importing executes SDK
code and needs its dependencies - and it never re-implements core-c: the
casing rule, the endpoints and the expanded tables all come out of
tools/reflect, which links the library itself.

Six checks, as designed:

  verb.new / verb.gone    python's module-level defs vs the STARKBANK_VERB_* lines
  field.new / .gone       python's __init__ parameters, camelCased by core-c,
    / .rename             vs the X-macro keys; a pair that differs only in
                          casing is a rename and is the dangerous one
  endpoint.changed        the docs method+path vs starkcore_api_endpoint + verb
  query.new / .gone       the docs endpoint parameters vs each verb's query keys
  flag.conflict           the REQUIRED and CREATE bits of the flags column vs
                          sdk-python's __init__ signature and the class
                          docstring's Parameters/Attributes sections. A HARD
                          STOP for the same reason type.conflict is: the bit
                          decides whether a create is validated here or 400s at
                          the API, so a divergence is adjudicated and signed,
                          not accepted in passing. PATCH has no python
                          counterpart and stays a human decision
  comment.stale           the header's field comment blocks vs the tables
  type.conflict           docs tag vs python check_* vs the table type. A HARD
                          STOP: it cannot be accepted into known-drift, because
                          when the inputs disagree a human decides which is right

plus resource.unjoined, which is not a table check but is what seeds
known-drift: a python package with no docs file, or the reverse.

Failure is on a CHANGE to the accepted set in tests/reference/known-drift.json,
never on the set being non-empty - a checker that is red on day one is a
checker nobody reads by week two. An accepted entry that stops firing is also a
failure, because a stale exemption is where a real divergence hides.
"""

import ast
import os
import re
import sys
import json
import glob
import subprocess


class DriftError(Exception):
    """An input this script cannot read. Never a finding: findings are data."""


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEADER = os.path.join(ROOT, "include", "starkbank.h")
REFLECT = os.path.join(ROOT, "tools", "reflect")
KNOWN = os.path.join(ROOT, "tests", "reference", "known-drift.json")
PINNED_SHA = os.path.join(ROOT, "tests", "reference", "sdk-python.sha")

# The sibling layout the Makefile already assumes, and which it passes in
# explicitly anyway. Relative, because a path to somebody's home directory does
# not belong in a repo that ships publicly.
DEFAULT_PYTHON = os.path.join(ROOT, os.pardir, "sdk-python", "starkbank")
DEFAULT_DOCS = os.path.join(ROOT, os.pardir, "app-docs", "src", "uipages", "api",
                            "bank", "resources")

FLAG_REQUIRED = 1
FLAG_CREATE = 2
FLAG_PATCH = 4

# A macro shape to the python verb it implements. NEW and PARAMS are local
# constructors - they allocate an empty entity and a query bag and never reach
# the API - so they have no python counterpart and are not compared.
VERB_PYTHON_NAME = {
    "POST_MULTI": "create",
    "POST_SINGLE": "create",
    "GET_ID": "get",
    "GET_FIRST": "get",
    "QUERY": "query",
    "PAGE": "page",
    "PATCH_ID": "update",
    "DELETE_ID": "delete",
}
VERB_NAMED = ("CONTENT", "CONTENT_INT", "SUB_RESOURCE")
VERB_LOCAL = ("NEW", "PARAMS")

# The docs' prose tags are lossy by construction: they have no date-vs-datetime,
# no seconds, and no sub-resource shape. This says only which tags CANNOT be the
# table's type, so the check fires on a real disagreement and not on the docs
# being coarse.
TAG_TYPES = {
    "STRING": ("STRING", "DATE", "DATETIME", "DATE_OR_DATETIME"),
    "INTEGER": ("AMOUNT", "NUMBER", "SECONDS", "RATE"),
    "FLOAT": ("RATE", "NUMBER", "AMOUNT"),
    "BOOLEAN": ("BOOL",),
    "LIST OF STRINGS": ("LIST_STRING",),
    "LIST OF OBJECTS": ("LIST_OBJECT", "LIST_RESOURCE"),
    "OBJECT": ("OBJECT", "RESOURCE"),
}

# python's input coercion is normative, and each of these means exactly one
# table type. The reverse matters as much: a table that says DATETIME where
# python does no coercion at all is a table that invented a semantic.
CHECK_TYPES = {
    "check_date": ("DATE", "DATE_OR_DATETIME"),
    "check_datetime": ("DATETIME",),
    "check_datetime_or_date": ("DATE_OR_DATETIME",),
    "check_timedelta": ("SECONDS",),
}
COERCED_TYPES = ("DATE", "DATETIME", "DATE_OR_DATETIME", "SECONDS")


def finding(check, resource, subject, detail):
    return {
        "key": "%s:%s:%s" % (check, resource, subject),
        "check": check,
        "resource": resource,
        "subject": subject,
        "detail": detail,
    }


# ------------------------------------------------------------------ the header

def fieldTypeNames(headerText):
    """STARKBANK_FIELD_* by value, so nothing here keeps a second copy of them."""
    names = {}
    for name, value in re.findall(r"^#define STARKBANK_FIELD_(\w+)\s+(\d+)", headerText, re.M):
        names[int(value)] = name
    if not names:
        raise DriftError("no STARKBANK_FIELD_* constants in the header")
    return names


TYPE_NAMES = None


def knownTypeNames():
    global TYPE_NAMES
    if TYPE_NAMES is None:
        with open(HEADER) as handle:
            TYPE_NAMES = frozenset(fieldTypeNames(handle.read()).values())
    return TYPE_NAMES


SECTION_RULE = re.compile(r"^/\* -{3,} ([A-Za-z][A-Za-z.]*) -*\*/\s*$", re.M)
SECTION_BANNER = re.compile(r"^/\* ={5,}\s*\n \*\s+([A-Za-z][A-Za-z.]*)\s*\n \* ={5,}", re.M)
NAME_TOKEN = re.compile(r"^[a-z][A-Za-z0-9]*[*+]{0,2}$")
TYPE_TOKEN = re.compile(r"^([A-Z][A-Z_]*)(?:\(\"([^\"]+)\"\))?$")


def sectionResourceName(title):
    """The header's section title as the resource name the tables use.

    A lowercase prefix is a python module path and names a resource of its own
    ("invoice.Log" is the resource "InvoiceLog"); a capitalised prefix is a
    sub-resource tag and keeps its dot ("Invoice.Rule").
    """
    if "." not in title:
        return title
    head, tail = title.split(".", 1)
    if head[:1].islower():
        return head[:1].upper() + head[1:] + tail
    return title


def undecorate(line):
    text = line.strip()
    for prefix in ("/*", "*/", "*"):
        if text.startswith(prefix):
            text = text[len(prefix):]
    if text.endswith("*/"):
        text = text[:-2]
    return text.strip()


def parseFieldSentence(sentence, resource):
    """A sentence of the Fields paragraph into {name: (type, required, patch, ro)}.

    Returns None when the sentence is not a field list at all, which is how the
    parser walks past the prose a comment block legitimately carries.
    """
    fields = {}
    refs = {}
    pending = []
    group = []
    for raw in sentence.replace(",", " ").replace(";", " ").split():
        token = raw.rstrip(".,;")
        if token == "":
            continue
        if token == "(ro)":
            if not group:
                return None
            for name in group:
                fields[name] = fields[name][:3] + (True,)
            continue
        typeMatch = TYPE_TOKEN.match(token)
        if typeMatch is not None:
            if not pending:
                return None
            typeName, ref = typeMatch.group(1), typeMatch.group(2)
            if typeName not in knownTypeNames():
                raise DriftError("%s: comment block names an unknown field type %s"
                                 % (resource, typeName))
            group = []
            for name in pending:
                bare = name.rstrip("*+")
                fields[bare] = (typeName, "*" in name, "+" in name, False)
                refs[bare] = ref
                group.append(bare)
            pending = []
            continue
        if NAME_TOKEN.match(token) is None:
            return None
        pending.append(token)
    if pending or not fields:
        return None
    return fields, refs


def parseCommentBlocks(headerText):
    """Every resource section's Fields paragraph and Query keys line.

    The blocks are the only documentation a consumer with no compiler ever
    reads, which is why they are parsed rather than trusted.
    """
    marks = [(match.start(), match.group(1)) for match in SECTION_RULE.finditer(headerText)]
    marks += [(match.start(), match.group(1)) for match in SECTION_BANNER.finditer(headerText)]
    marks.sort()

    blocks = {}
    for index, (start, title) in enumerate(marks):
        if title[:1].islower() and "." not in title:
            continue                      # an engine section: entity, list, iter
        end = marks[index + 1][0] if index + 1 < len(marks) else len(headerText)
        block = parseOneBlock(headerText[start:end], sectionResourceName(title))
        if block is not None:
            blocks[sectionResourceName(title)] = block
    return blocks


def parseOneBlock(text, resource):
    lines = [undecorate(line) for line in text.split("\n")]
    collected = []
    queryKeys = []
    started = False
    for line in lines:
        if line.startswith("Query keys"):
            queryKeys = [key.strip(" .") for key in line.split(":", 1)[1].split(",")]
            break
        if line.startswith("Fields"):
            started = True
            line = re.sub(r"^Fields\s*(\([^)]*\))?\s*:?", "", line)
        if not started:
            continue
        if line.startswith("#") or line.startswith("STARKBANK_API"):
            break
        collected.append(line)
    if not started:
        return None

    fields = {}
    refs = {}
    seen = False
    for sentence in re.split(r"(?<=\.)\s+", " ".join(collected)):
        parsed = parseFieldSentence(sentence, resource)
        if parsed is None:
            if seen:
                break
            continue
        seen = True
        fields.update(parsed[0])
        refs.update(parsed[1])
    if not fields:
        return None
    return {"fields": fields, "refs": refs, "queryKeys": queryKeys}


# ---------------------------------------------------------------- the C tables

RESOURCE_MACRO = re.compile(r"STARKBANK_RESOURCE(?:_FULL)?\(\s*(\w+)\s*,\s*\"([^\"]+)\"")
VERB_MACRO = re.compile(r"STARKBANK_VERB_([A-Z_]+)\(\s*(\w+)\s*(?:,\s*([A-Za-z_]\w*))?")


def parseVerbs(sourceText):
    """{resource name: {"ident":…, "verbs": {python verb name: macro shape}}}."""
    resources = {}
    identNames = {}
    for ident, name in RESOURCE_MACRO.findall(sourceText):
        identNames[ident] = name
        resources[name] = {"ident": ident, "verbs": {}}
    for shape, ident, argument in VERB_MACRO.findall(sourceText):
        if shape in VERB_LOCAL:
            continue
        name = identNames.get(ident)
        if name is None:
            raise DriftError("verb macro for unregistered %s" % ident)
        if shape in VERB_NAMED:
            resources[name]["verbs"][argument] = shape
            continue
        if shape not in VERB_PYTHON_NAME:
            raise DriftError("unknown verb macro STARKBANK_VERB_%s" % shape)
        resources[name]["verbs"][VERB_PYTHON_NAME[shape]] = shape
    return resources


HANDWRITTEN = re.compile(r"^STARKBANK_API\s+.*?STARKBANK_CALL\s+(starkbank_\w+)\(", re.M)


def readVerbs(root):
    resources = {}
    for path in sorted(glob.glob(os.path.join(root, "starkbank", "*", "*.c"))):
        with open(path) as handle:
            for name, entry in parseVerbs(handle.read()).items():
                resources.setdefault(name, {"ident": entry["ident"], "verbs": {}})
                resources[name]["verbs"].update(entry["verbs"])

    # A judgement method is declared in the header and defined only by hand, so
    # it appears in no macro and must be joined back by its symbol name.
    byIdent = dict((entry["ident"], name) for name, entry in resources.items())
    for path in sorted(glob.glob(os.path.join(root, "handwritten", "*.c"))):
        with open(path) as handle:
            for symbol in HANDWRITTEN.findall(handle.read()):
                tail = symbol[len("starkbank_"):]
                owner = max((ident for ident in byIdent if tail.startswith(ident + "_")),
                            key=len, default=None)
                if owner is None:
                    raise DriftError("hand-written %s belongs to no resource" % symbol)
                resources[byIdent[owner]]["verbs"][tail[len(owner) + 1:]] = "HANDWRITTEN"
    return resources


def readTables():
    if not os.path.exists(REFLECT):
        raise DriftError("tools/reflect is not built; run make reflect")
    output = subprocess.check_output([REFLECT]).decode("utf-8")
    tables = {}
    for resource in json.loads(output)["resources"]:
        tables[resource["name"]] = resource
    return tables


def camelize(names):
    """python snake_case to wire camelCase, through core-c's own converter."""
    names = [name for name in names if name]
    if not names:
        return {}
    process = subprocess.Popen([REFLECT, "--camel"], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE)
    output = process.communicate(("\n".join(names) + "\n").encode("utf-8"))[0]
    converted = output.decode("utf-8").split()
    if len(converted) != len(names):
        raise DriftError("reflect --camel returned %d of %d names"
                         % (len(converted), len(names)))
    return dict(zip(names, converted))


# ---------------------------------------------------------------- sdk-python

def dictValue(node, key):
    for item, value in zip(node.keys, node.values):
        if isinstance(item, ast.Constant) and item.value == key:
            return value
    return None


def classNode(tree, className):
    for node in ast.walk(tree):
        if isinstance(node, ast.ClassDef) and node.name == className:
            return node
    return None


def classInit(tree, className):
    node = classNode(tree, className)
    if node is None:
        return None
    for item in node.body:
        if isinstance(item, ast.FunctionDef) and item.name == "__init__":
            return item
    return None


# The two docstring sections that say what a field is FOR. sdk-python writes
# them by hand next to the signature, so they carry the one fact the signature
# cannot: a parameter with a default is still a create parameter, while a
# return-only attribute is not, and both are spelled `x=None`.
DOC_SECTION = re.compile(r"^\s*##\s*(Parameters|Attributes)\s*\(([^)]*)\)\s*:")
DOC_FIELD = re.compile(r"^\s*-\s*([A-Za-z_][A-Za-z0-9_]*)\s*\[")

SECTION_CREATE = "create"
SECTION_REQUIRED = "required"
SECTION_RETURN = "return"


def docstringSections(node):
    """field -> one of required / create / return, from the class docstring."""
    text = ast.get_docstring(node) or ""
    sections = {}
    current = None
    for line in text.splitlines():
        header = DOC_SECTION.match(line)
        if header is not None:
            kind = header.group(2).strip().lower()
            if header.group(1) == "Attributes":
                current = SECTION_RETURN
            elif kind == "required":
                current = SECTION_REQUIRED
            else:
                # "optional" and "conditionally required" alike: creatable, and
                # not something this checker calls required on its own.
                current = SECTION_CREATE
            continue
        if line.strip().startswith("##"):
            current = None
            continue
        field = DOC_FIELD.match(line)
        if field is None or current is None:
            continue
        sections[field.group(1)] = current
    return sections


def signatureRequired(function):
    """Parameters with no default: python's own statement of requiredness."""
    arguments = [argument.arg for argument in function.args.args]
    positional = arguments[:len(arguments) - len(function.args.defaults)]
    return set(name for name in positional if name != "self")


def initFields(function):
    arguments = [argument.arg for argument in function.args.args if argument.arg != "self"]
    checks = {}
    for node in ast.walk(function):
        if not isinstance(node, ast.Assign) or not isinstance(node.value, ast.Call):
            continue
        target = node.targets[0]
        if not isinstance(target, ast.Attribute):
            continue
        callee = node.value.func
        name = callee.id if isinstance(callee, ast.Name) else getattr(callee, "attr", None)
        if name is not None and name.startswith("check_"):
            checks[target.attr] = name
    return arguments, checks


def readPin():
    """The pinned sdk-python commit, comments stripped."""
    with open(PINNED_SHA) as handle:
        for line in handle:
            line = line.split("#", 1)[0].strip()
            if line:
                return line
    raise DriftError("no sha in %s" % PINNED_SHA)


def observedSha(root):
    """The commit the sdk-python on this machine is actually at, or None."""
    try:
        output = subprocess.check_output(["git", "-C", root, "rev-parse", "HEAD"],
                                         stderr=subprocess.DEVNULL)
    except (OSError, subprocess.CalledProcessError):
        return None
    return output.decode("utf-8").strip()


def readPython(root):
    """Every resource sdk-python declares, read with ast and never imported."""
    modules = []
    for path in sorted(glob.glob(os.path.join(root, "**", "*.py"), recursive=True)):
        with open(path) as handle:
            tree = ast.parse(handle.read(), path)
        for node in tree.body:
            if not isinstance(node, ast.Assign) or not isinstance(node.value, ast.Dict):
                continue
            target = node.targets[0]
            if getattr(target, "id", None) not in ("_resource", "_sub_resource"):
                continue
            name = dictValue(node.value, "name")
            className = dictValue(node.value, "class")
            if name is None or className is None:
                continue
            modules.append((path, target.id, name.value, className.id, tree))

    # A sub-resource's python name is bare ("Rule"), and two of them collide
    # across resources, so it is qualified by the resource its package owns -
    # exactly the tag the tables carry.
    owners = {}
    for path, kind, name, _, _ in modules:
        if kind == "_resource" and os.path.dirname(os.path.dirname(path)) == root.rstrip("/"):
            owners.setdefault(os.path.basename(os.path.dirname(path)), name)

    resources = {}
    for path, kind, name, className, tree in modules:
        key = name
        if kind == "_sub_resource":
            package = os.path.relpath(path, root).split(os.sep)[0]
            key = "%s.%s" % (owners.get(package, package.capitalize()), name)
        function = classInit(tree, className)
        if function is None:
            raise DriftError("%s: no __init__ for %s" % (path, className))
        fields, checks = initFields(function)
        verbs = set(node.name for node in tree.body
                    if isinstance(node, ast.FunctionDef) and not node.name.startswith("_"))
        resources[key] = {"module": path, "fields": fields, "checks": checks, "verbs": verbs,
                          "sections": docstringSections(classNode(tree, className)),
                          "required": signatureRequired(function)}
    return resources


# --------------------------------------------------------------- the docs JSON

def readDocs(directory):
    endpoints = {}
    objects = {}
    slugs = set()
    for path in sorted(glob.glob(os.path.join(directory, "*.json"))):
        with open(path) as handle:
            document = json.load(handle)
        slug = os.path.basename(path)[:-len(".json")]
        slugs.add(slug)
        for endpoint in document.get("endpoints", []):
            # A few doc entries are prose sections with a codeSampleKey and no
            # route at all (corporate-rule has three). They document behaviour,
            # not an endpoint, and joining on a missing key would crash the
            # checker on a file nobody touched.
            if "method" not in endpoint or "path" not in endpoint:
                continue
            key = (endpoint["method"], endpoint["path"])
            endpoints[key] = [parameter["name"] for parameter in endpoint.get("parameters", [])]
        for parameter in document.get("object", {}).get("parameters", []):
            objects.setdefault(document.get("title", slug), {})[parameter["name"]] = parameter["tag"]
    return {"endpoints": endpoints, "objects": objects, "slugs": slugs}


def verbEndpoint(shape, endpoint, verbName):
    """The (method, path) a verb macro reaches, from core-c's endpoint alone."""
    base = "/v2/" + endpoint
    if shape in ("POST_MULTI", "POST_SINGLE"):
        return ("POST", base)
    if shape in ("GET_FIRST", "QUERY", "PAGE"):
        return ("GET", base)
    if shape == "GET_ID":
        return ("GET", base + "/:id")
    if shape == "PATCH_ID":
        return ("PATCH", base + "/:id")
    if shape == "DELETE_ID":
        return ("DELETE", base + "/:id")
    if shape in ("CONTENT", "CONTENT_INT", "SUB_RESOURCE"):
        return ("GET", base + "/:id/" + verbName)
    return None


def tagFitsType(tag, typeName):
    return typeName in TAG_TYPES.get(tag, ())


def checkFitsType(checkName, typeName):
    if checkName is None:
        return typeName not in COERCED_TYPES
    return typeName in CHECK_TYPES.get(checkName, ())


# ------------------------------------------------------------------- checks

def checkVerbs(verbs, python, findings):
    for name in sorted(verbs):
        if name not in python:
            findings.append(finding("verb.gone", name, "*",
                                    "sdk-c implements a resource sdk-python does not declare"))
            continue
        ours = set(verbs[name]["verbs"])
        theirs = python[name]["verbs"]
        for verb in sorted(theirs - ours):
            findings.append(finding("verb.new", name, verb,
                                    "sdk-python has %s() and sdk-c does not" % verb))
        for verb in sorted(ours - theirs):
            findings.append(finding("verb.gone", name, verb,
                                    "sdk-c declares %s and sdk-python has no such def; "
                                    "removing it is an ABI break" % verb))


def checkFields(tables, python, findings):
    for name in sorted(tables):
        if name not in python:
            continue
        camel = camelize(python[name]["fields"])
        theirs = dict((camel[field], field) for field in python[name]["fields"])
        ours = set(field["key"] for field in tables[name]["fields"])
        new = sorted(set(theirs) - ours)
        gone = sorted(ours - set(theirs))

        # A rename that only changes casing is the one drift no fixture can
        # catch: the goldens are recorded from python, so they go stale in
        # lockstep with the tables and agree with each other while both
        # disagree with the API.
        renamed = {}
        for key in list(new):
            flat = key.lower()
            for other in list(gone):
                if other.lower() != flat:
                    continue
                renamed[other] = key
                new.remove(key)
                gone.remove(other)
                break
        for old, key in sorted(renamed.items()):
            findings.append(finding("field.rename", name, old,
                                    "the table says %s and core-c casing of python's %s gives %s"
                                    % (old, theirs[key], key)))
        for key in new:
            findings.append(finding("field.new", name, key,
                                    "sdk-python carries %s and the table does not" % theirs[key]))
        for key in gone:
            findings.append(finding("field.gone", name, key,
                                    "the table carries %s and sdk-python does not" % key))


def checkEndpoints(tables, verbs, docs, findings):
    for name in sorted(verbs):
        if "." in name or name not in tables:
            continue                       # a sub-resource has no endpoint of its own
        endpoint = tables[name]["endpoint"]
        if endpoint is None:
            findings.append(finding("endpoint.changed", name, "*",
                                    "starkcore_api_endpoint refuses the resource name"))
            continue
        for verbName, shape in sorted(verbs[name]["verbs"].items()):
            route = verbEndpoint(shape, endpoint, verbName)
            if route is None or route in docs["endpoints"]:
                continue
            findings.append(finding("endpoint.changed", name, verbName,
                                    "%s %s is not in the docs endpoint list" % route))


def checkQueries(tables, verbs, docs, findings):
    for name in sorted(verbs):
        if "." in name or name not in tables:
            continue
        shapes = set(verbs[name]["verbs"].values())
        if not shapes & set(("QUERY", "PAGE")):
            continue
        route = ("GET", "/v2/" + tables[name]["endpoint"])
        if route not in docs["endpoints"]:
            continue
        # cursor is pagination, which is core-c's, and fields is a projection
        # this tier does not offer; neither is a filter the table should name.
        documented = set(docs["endpoints"][route]) - set(("cursor", "fields"))
        ours = set(tables[name]["queryKeys"])
        for key in sorted(documented - ours):
            findings.append(finding("query.new", name, key,
                                    "the docs list %s as a filter and the table rejects it" % key))
        for key in sorted(ours - documented):
            findings.append(finding("query.gone", name, key,
                                    "the table accepts %s and the docs do not list it" % key))


def checkComments(tables, blocks, findings):
    for name in sorted(tables):
        if name not in blocks:
            findings.append(finding("comment.stale", name, "*",
                                    "no field comment block in the public header"))
            continue
        block = blocks[name]
        ours = dict((field["key"], field) for field in tables[name]["fields"])
        for key in sorted(set(block["fields"]) - set(ours)):
            findings.append(finding("comment.stale", name, key,
                                    "the comment describes %s and the table has no such field" % key))
        for key in sorted(set(ours) - set(block["fields"])):
            findings.append(finding("comment.stale", name, key,
                                    "the table has %s and the comment does not mention it" % key))
        for key in sorted(set(ours) & set(block["fields"])):
            described = block["fields"][key]
            field = ours[key]
            actual = TYPE_NAMES_BY_VALUE[field["type"]]
            if described[0] != actual:
                findings.append(finding("comment.stale", name, key,
                                        "the comment says %s and the table says %s"
                                        % (described[0], actual)))
                continue
            if described[1] and not field["flags"] & FLAG_REQUIRED:
                findings.append(finding("comment.stale", name, key,
                                        "the comment marks %s required on create" % key))
            if described[2] and not field["flags"] & FLAG_PATCH:
                findings.append(finding("comment.stale", name, key,
                                        "the comment marks %s patchable" % key))
            if described[3] != (field["flags"] == 0):
                findings.append(finding("comment.stale", name, key,
                                        "the comment and the table disagree on %s being read-only"
                                        % key))
        documented = [key for key in block["queryKeys"] if key]
        if documented and documented != list(tables[name]["queryKeys"]):
            findings.append(finding("comment.stale", name, "queryKeys",
                                    "the comment lists %s and the table lists %s"
                                    % (", ".join(documented), ", ".join(tables[name]["queryKeys"]))))


def checkTypes(tables, python, docs, findings):
    """The hard stop. Three inputs, and no table change lands while they disagree."""
    for name in sorted(tables):
        documented = docs["objects"].get(name, {})
        checks = python.get(name, {}).get("checks", {})
        camel = camelize(list(checks))
        for field in tables[name]["fields"]:
            key = field["key"]
            typeName = TYPE_NAMES_BY_VALUE[field["type"]]
            tag = documented.get(key)
            if tag is not None and not tagFitsType(tag, typeName):
                findings.append(finding("type.conflict", name, key,
                                        "the docs tag %s and the table type %s cannot both be right"
                                        % (tag, typeName)))
            coercion = None
            for snake, checkName in checks.items():
                if camel[snake] == key:
                    coercion = checkName
            if not checkFitsType(coercion, typeName):
                findings.append(finding("type.conflict", name, key,
                                        "sdk-python coerces with %s and the table says %s"
                                        % (coercion, typeName)))


def checkFlags(tables, python, findings):
    """The flags column against python, because a comment cannot guard it.

    Two of the three bits are derivable and therefore checked. CREATE is the
    docstring's section: a create parameter and a return-only attribute are
    both spelled `x=None` in a signature, so nothing but the prose next to it
    can tell them apart. REQUIRED is the docstring's "(required)" section AND
    a parameter with no default, and the two disagreeing is itself a finding.

    Requiredness is only read off the signature for fields the docstring calls
    create parameters. A return-only object is built by from_api_json and takes
    every attribute positionally - invoice.Log is `__init__(self, id, created,
    type, errors, invoice)` - so positional-ness there says how python parses a
    response, not what a caller must supply.

    PATCH is deliberately not derived. sdk-python's update() takes **patch and
    names nothing, so there is no python fact to compare against; it stays a
    human decision, and saying so here is the point of the check.
    """
    for name in sorted(tables):
        if name not in python:
            continue
        sections = python[name]["sections"]
        camel = camelize(list(sections) + list(python[name]["required"]))
        documented = dict((camel[field], kind) for field, kind in sections.items())
        positional = set(camel[field] for field in python[name]["required"])

        for key in sorted(positional):
            if documented.get(key) != SECTION_CREATE:
                continue
            findings.append(finding("flag.conflict", name, key,
                                    "sdk-python's signature gives %s no default and its own "
                                    "docstring lists it as optional" % key))

        for field in tables[name]["fields"]:
            key = field["key"]
            kind = documented.get(key)
            if kind is None:
                continue                   # field.new / field.gone owns this
            ours = bool(field["flags"] & FLAG_CREATE)
            theirs = kind != SECTION_RETURN
            if ours != theirs:
                findings.append(finding("flag.conflict", name, key,
                                        "the table %s CREATE and sdk-python documents %s as %s"
                                        % ("sets" if ours else "does not set", key,
                                           "return-only" if kind == SECTION_RETURN
                                           else "a create parameter")))
                continue
            if kind == SECTION_RETURN:
                continue
            ours = bool(field["flags"] & FLAG_REQUIRED)
            theirs = kind == SECTION_REQUIRED and key in positional
            if ours != theirs:
                findings.append(finding("flag.conflict", name, key,
                                        "the table %s REQUIRED and sdk-python %s"
                                        % ("sets" if ours else "does not set",
                                           "does" if theirs else "does not")))


def checkUnjoined(pythonRoot, docs, findings):
    """What seeds known-drift: names that do not join across the two inputs.

    Not a table check - it names surface that does not exist here yet - but it
    is measured with the same tools and is red the day a resource is documented
    and not implemented in python, or the reverse.
    """
    packages = set(name for name in os.listdir(pythonRoot)
                   if os.path.isdir(os.path.join(pythonRoot, name))
                   and not name.startswith("_") and name != "utils"
                   and glob.glob(os.path.join(pythonRoot, name, "*.py")))
    flattened = dict((slug.replace("-", ""), slug) for slug in docs["slugs"])
    for package in sorted(packages - set(flattened)):
        findings.append(finding("resource.unjoined", package, "docs",
                                "sdk-python has the package and the docs have no resource file"))
    for flat in sorted(set(flattened) - packages):
        findings.append(finding("resource.unjoined", flattened[flat], "python",
                                "the docs document the resource and sdk-python has no package"))


# ------------------------------------------------------------------ reconcile

RESOLUTIONS = ("python", "docs", "table")

# A finding that a plain known-drift entry cannot silence. Both of these say
# two sources of truth disagree, and both decide how money is validated, so
# clearing one costs a named decision rather than one more line in a list.
HARD_STOP_CHECKS = ("type.conflict", "flag.conflict")


def reconcile(findings, known):
    """Fail on a CHANGE to the accepted set, in either direction."""
    report = []
    accepted = {}
    for entry in known.get("accepted", []):
        if not entry.get("owner") or not entry.get("reason"):
            report.append("known-drift: %s has no owner or no reason" % entry.get("key"))
        accepted[entry["key"]] = entry

    found = dict((item["key"], item) for item in findings)
    for key in sorted(found):
        if found[key]["check"] in HARD_STOP_CHECKS:
            # The hard stop, kept hard: an ordinary exemption does not silence
            # it. Only an entry that names which input won - and is therefore a
            # decision somebody made and signed - does, and the same entry goes
            # stale the moment the conflict is fixed upstream.
            resolution = accepted.get(key, {}).get("resolution")
            if resolution in RESOLUTIONS:
                continue
            report.append("HARD STOP %s: %s" % (key, found[key]["detail"]))
            report.append("  a %s is not accepted like other drift. Resolve it, or record a "
                          "resolution of %s in known-drift.json saying which input wins and why"
                          % (found[key]["check"], "/".join(RESOLUTIONS)))
            continue
        if key not in accepted:
            report.append("NEW %s: %s" % (key, found[key]["detail"]))
    for key in sorted(set(accepted) - set(found)):
        report.append("STALE %s: accepted in known-drift.json and no longer fires; "
                      "remove the entry" % key)
    return (1 if report else 0, "\n".join(report))


def main(argv):
    pythonRoot = os.environ.get("SDK_PYTHON", DEFAULT_PYTHON)
    docsRoot = os.environ.get("APP_DOCS", DEFAULT_DOCS)
    for argument in argv[1:]:
        if argument.startswith("--python="):
            pythonRoot = argument.split("=", 1)[1]
        elif argument == "--require-pin":
            continue
        elif argument.startswith("--docs="):
            docsRoot = argument.split("=", 1)[1]
        else:
            sys.stderr.write("usage: drift.py [--python=DIR] [--docs=DIR] [--require-pin]\n")
            return 2

    global TYPE_NAMES_BY_VALUE
    with open(HEADER) as handle:
        headerText = handle.read()
    TYPE_NAMES_BY_VALUE = fieldTypeNames(headerText)

    missing = [path for path in (pythonRoot, docsRoot) if not os.path.isdir(path)]
    if missing:
        sys.stderr.write("drift: cannot read %s\n" % ", ".join(missing))
        return 2

    # The pin is advisory locally and required for a release: a developer with
    # a branch checked out must still be able to run the checker, and a tag
    # must not be cut against an unknown upstream.
    pinned = readPin()
    observed = observedSha(pythonRoot)
    if observed is not None and observed != pinned:
        message = ("drift: sdk-python is at %s and tests/reference/sdk-python.sha pins %s\n"
                   % (observed[:12], pinned[:12]))
        if "--require-pin" in argv[1:]:
            sys.stderr.write(message + "drift: --require-pin, refusing to compare\n")
            return 2
        sys.stderr.write(message)

    tables = readTables()
    verbs = readVerbs(ROOT)
    python = readPython(pythonRoot)
    docs = readDocs(docsRoot)
    blocks = parseCommentBlocks(headerText)

    findings = []
    checkVerbs(verbs, python, findings)
    checkFields(tables, python, findings)
    checkEndpoints(tables, verbs, docs, findings)
    checkQueries(tables, verbs, docs, findings)
    checkComments(tables, blocks, findings)
    checkTypes(tables, python, docs, findings)
    checkFlags(tables, python, findings)
    checkUnjoined(pythonRoot, docs, findings)

    with open(KNOWN) as handle:
        known = json.load(handle)
    status, report = reconcile(findings, known)
    if status != 0:
        sys.stderr.write(report + "\n")
        sys.stderr.write("drift: %d findings, %d accepted in %s\n"
                         % (len(findings), len(known.get("accepted", [])),
                            os.path.relpath(KNOWN, ROOT)))
        return 1
    sys.stdout.write("check-drift: %d resources, %d findings, all %d accepted in known-drift.json\n"
                     % (len(tables), len(findings), len(known.get("accepted", []))))
    return 0


TYPE_NAMES_BY_VALUE = {}

if __name__ == "__main__":
    sys.exit(main(sys.argv))
