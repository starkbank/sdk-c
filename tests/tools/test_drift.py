"""Unit cases for tools/drift.py.

The drift checker is the only guard between the field tables and reality, and
it is a Python script in a C repo, so the parts of it that can be wrong
silently - the comment micro-format, the verb reader, the python AST reader
and the known-drift set comparison - are tested here rather than trusted.

Run: python3 tests/tools/test_drift.py
"""

import os
import sys
import json
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import drift


class CommentBlocks(unittest.TestCase):

    def testReadsAMultiLineBlockWithSharedTypes(self):
        header = """
/* ======================================================
 *                        Widget
 * ======================================================
 *
 * Fields (wire keys; * = required on create, + = also patchable).
 *   amount*+ AMOUNT            taxId* STRING
 *   rules LIST_RESOURCE("Widget.Rule")
 *   id pdf STRING (ro)         created updated DATETIME (ro)
 *
 * Query keys: limit, after, ids.
 */
#define STARKBANK_WIDGET_AMOUNT "amount"
"""
        blocks = drift.parseCommentBlocks(header)
        self.assertEqual(sorted(blocks["Widget"]["fields"]),
                         ["amount", "created", "id", "pdf", "rules", "taxId", "updated"])
        self.assertEqual(blocks["Widget"]["fields"]["amount"], ("AMOUNT", True, True, False))
        self.assertEqual(blocks["Widget"]["fields"]["rules"][0], "LIST_RESOURCE")
        self.assertEqual(blocks["Widget"]["fields"]["created"], ("DATETIME", False, False, True))
        self.assertEqual(blocks["Widget"]["queryKeys"], ["limit", "after", "ids"])

    def testStopsAtTheFirstSentenceThatIsNotAFieldSpec(self):
        """A prose sentence after the list must not become a field.

        Transfer.Rule's block ends "ex: key "resendingLimit", value 5", and a
        parser that kept reading would invent a field called ex.
        """
        header = """
/* ---------------------------------------------- Widget.Rule */
/* Fields: key* STRING, value* NUMBER. ex: key "resendingLimit", value 5.
   Note the value type. */
#define STARKBANK_WIDGET_RULE_KEY "key"
"""
        blocks = drift.parseCommentBlocks(header)
        self.assertEqual(sorted(blocks["Widget.Rule"]["fields"]), ["key", "value"])

    def testALowercaseSectionPrefixNamesAConcatenatedResource(self):
        """"invoice.Log" in the header is the resource "InvoiceLog"; the
        capitalised "Invoice.Rule" is a sub-resource tag and keeps its dot."""
        header = """
/* ---------------------------------------------- widget.Log */
/* Fields: id STRING (ro), created DATETIME (ro). */
#define X 1
"""
        self.assertIn("WidgetLog", drift.parseCommentBlocks(header))

    def testAnUnparsableFieldListIsAnError(self):
        header = """
/* ---------------------------------------------- Widget */
/* Fields: amount NOTATYPE. */
#define X 1
"""
        with self.assertRaises(drift.DriftError):
            drift.parseCommentBlocks(header)


class Verbs(unittest.TestCase):

    def testMapsEveryMacroToItsPythonVerbName(self):
        source = """
STARKBANK_RESOURCE(widget, "Widget", STARKBANK_WIDGET_FIELDS, widgetQuery);
STARKBANK_VERB_NEW(widget)
STARKBANK_VERB_PARAMS(widget)
STARKBANK_VERB_POST_MULTI(widget)
STARKBANK_VERB_GET_ID(widget)
STARKBANK_VERB_QUERY(widget)
STARKBANK_VERB_PAGE(widget)
STARKBANK_VERB_PATCH_ID(widget)
STARKBANK_VERB_DELETE_ID(widget)
STARKBANK_VERB_CONTENT(widget, pdf)
STARKBANK_VERB_CONTENT_INT(widget, qrcode, "size", 1, 50)
STARKBANK_VERB_SUB_RESOURCE(widget, payment, "Payment", "Widget.Payment")
"""
        verbs = drift.parseVerbs(source)
        self.assertEqual(verbs["Widget"]["verbs"], {
            "create": "POST_MULTI", "get": "GET_ID", "query": "QUERY", "page": "PAGE",
            "update": "PATCH_ID", "delete": "DELETE_ID", "pdf": "CONTENT",
            "qrcode": "CONTENT_INT", "payment": "SUB_RESOURCE"})

    def testResourceFullIsReadLikeResource(self):
        source = 'STARKBANK_RESOURCE_FULL(widget, "Widget", F, q, refFn);\nSTARKBANK_VERB_GET_ID(widget)\n'
        self.assertEqual(drift.parseVerbs(source)["Widget"]["verbs"], {"get": "GET_ID"})

    def testGetFirstIsPythonsGet(self):
        source = 'STARKBANK_RESOURCE(balance, "Balance", F, NULL);\nSTARKBANK_VERB_GET_FIRST(balance)\n'
        self.assertEqual(drift.parseVerbs(source)["Balance"]["verbs"], {"get": "GET_FIRST"})


class Python(unittest.TestCase):

    def testReadsFieldsChecksAndVerbsWithoutImporting(self):
        with tempfile.TemporaryDirectory() as directory:
            package = os.path.join(directory, "widget")
            os.makedirs(package)
            with open(os.path.join(package, "__widget.py"), "w") as handle:
                handle.write(
                    "from starkcore.utils.checks import check_datetime\n"
                    "class Widget(Resource):\n"
                    "    def __init__(self, amount, tax_id, due=None, id=None):\n"
                    "        Resource.__init__(self, id=id)\n"
                    "        self.amount = amount\n"
                    "        self.tax_id = tax_id\n"
                    "        self.due = check_datetime_or_date(due)\n"
                    "_resource = {\"class\": Widget, \"name\": \"Widget\"}\n"
                    "def create(widgets, user=None):\n    pass\n"
                    "def _private(x):\n    pass\n")
            resources = drift.readPython(directory)
        self.assertEqual(resources["Widget"]["fields"], ["amount", "tax_id", "due", "id"])
        self.assertEqual(resources["Widget"]["checks"]["due"], "check_datetime_or_date")
        self.assertEqual(resources["Widget"]["verbs"], {"create"})

    def testSubResourceIsQualifiedByItsOwningResource(self):
        with tempfile.TemporaryDirectory() as directory:
            package = os.path.join(directory, "widget")
            os.makedirs(os.path.join(package, "rule"))
            with open(os.path.join(package, "__widget.py"), "w") as handle:
                handle.write("class Widget(object):\n    def __init__(self, id=None):\n"
                             "        self.id = id\n"
                             "_resource = {\"class\": Widget, \"name\": \"Widget\"}\n")
            with open(os.path.join(package, "rule", "__rule.py"), "w") as handle:
                handle.write("class Rule(object):\n    def __init__(self, key, value):\n"
                             "        self.key = key\n        self.value = value\n"
                             "_sub_resource = {\"class\": Rule, \"name\": \"Rule\"}\n")
            resources = drift.readPython(directory)
        self.assertEqual(resources["Widget.Rule"]["fields"], ["key", "value"])


class KnownDrift(unittest.TestCase):

    def findings(self):
        return [drift.finding("field.new", "Widget", "colour", "python has it, the table does not")]

    def testAnUnacceptedFindingFails(self):
        status, report = drift.reconcile(self.findings(), {"accepted": []})
        self.assertEqual(status, 1)
        self.assertIn("field.new:Widget:colour", report)

    def testAnAcceptedFindingPasses(self):
        accepted = {"accepted": [{"key": "field.new:Widget:colour", "owner": "me", "reason": "why"}]}
        status, _ = drift.reconcile(self.findings(), accepted)
        self.assertEqual(status, 0)

    def testAnAcceptedEntryThatNoLongerFiresFails(self):
        """Fail on set CHANGE, not on non-emptiness: a stale exemption is how a
        real divergence hides behind an old one."""
        accepted = {"accepted": [{"key": "field.gone:Widget:size", "owner": "me", "reason": "why"}]}
        status, report = drift.reconcile([], accepted)
        self.assertEqual(status, 1)
        self.assertIn("no longer fires", report)

    def testAnEntryWithoutAnOwnerFails(self):
        accepted = {"accepted": [{"key": "field.new:Widget:colour", "owner": "", "reason": "why"}]}
        status, report = drift.reconcile(self.findings(), accepted)
        self.assertEqual(status, 1)
        self.assertIn("owner", report)

    def testATypeConflictIsSilencedOnlyByARecordedAdjudication(self):
        """The hard stop stays a hard stop, but it must be resolvable: an entry
        that names which input won, and why, is a human decision written down.
        An entry without one is an exemption, and an exemption is what the hard
        stop exists to refuse."""
        conflicts = [drift.finding("type.conflict", "Widget", "amount", "docs says STRING")]
        resolved = {"accepted": [{"key": "type.conflict:Widget:amount", "owner": "me",
                                  "reason": "the docs tag is lossy", "resolution": "python"}]}
        status, report = drift.reconcile(conflicts, resolved)
        self.assertEqual(status, 0, report)

    def testAnAdjudicationMustNameAKnownInput(self):
        conflicts = [drift.finding("type.conflict", "Widget", "amount", "docs says STRING")]
        resolved = {"accepted": [{"key": "type.conflict:Widget:amount", "owner": "me",
                                  "reason": "r", "resolution": "whatever"}]}
        status, report = drift.reconcile(conflicts, resolved)
        self.assertEqual(status, 1)
        self.assertIn("resolution", report)

    def testAStaleAdjudicationFailsLikeAnyOtherStaleEntry(self):
        resolved = {"accepted": [{"key": "type.conflict:Widget:amount", "owner": "me",
                                  "reason": "r", "resolution": "python"}]}
        status, report = drift.reconcile([], resolved)
        self.assertEqual(status, 1)
        self.assertIn("no longer fires", report)

    def testATypeConflictCannotBeAccepted(self):
        """type.conflict is a hard stop by design: a human resolves it, an
        exemption does not."""
        conflicts = [drift.finding("type.conflict", "Widget", "amount", "docs says STRING")]
        accepted = {"accepted": [{"key": "type.conflict:Widget:amount", "owner": "me", "reason": "r"}]}
        status, report = drift.reconcile(conflicts, accepted)
        self.assertEqual(status, 1)
        self.assertIn("HARD STOP", report)


class Flags(unittest.TestCase):
    """The flags column, which before this check was guarded by a comment.

    camelize shells out to tools/reflect for core-c's own casing rule; these
    cases are about the comparison, so the names are already camel and the
    converter is stubbed to the identity.
    """

    INVOICE = """# Invoice object
    ## Parameters (required):
    - amount [integer]: value in cents. ex: 1234
    - taxId [string]: payer tax ID. ex: "01234567890"
    ## Parameters (optional):
    - fine [float, default 2.0]: fine in %. ex: 2.5
    ## Parameters (conditionally required):
    - due [datetime]: due date.
    ## Attributes (return-only):
    - brcode [string]: BR Code for the payment.
    ## Return:
    - list of Invoice objects
    """

    def setUp(self):
        self.camelize = drift.camelize
        drift.camelize = lambda names: dict((name, name) for name in names)

    def tearDown(self):
        drift.camelize = self.camelize

    @staticmethod
    def sections(text):
        import ast as astModule
        tree = astModule.parse("class Invoice(object):\n    \"\"\"%s\"\"\"\n" % text)
        return drift.docstringSections(tree.body[0])

    @staticmethod
    def python(sections, required, name="Invoice"):
        return {name: {"sections": sections, "required": set(required)}}

    @staticmethod
    def table(fields, name="Invoice"):
        return {name: {"fields": [{"key": key, "type": 0, "flags": flags, "ref": None}
                                  for key, flags in fields]}}

    def testReadsEverySectionHeadingSdkPythonUses(self):
        sections = self.sections(self.INVOICE)
        self.assertEqual(sections["amount"], drift.SECTION_REQUIRED)
        self.assertEqual(sections["taxId"], drift.SECTION_REQUIRED)
        self.assertEqual(sections["fine"], drift.SECTION_CREATE)
        self.assertEqual(sections["brcode"], drift.SECTION_RETURN)
        self.assertNotIn("list", sections)

    def testConditionallyRequiredIsCreatableAndNotRequired(self):
        """python says "conditionally required" nine times. The condition is
        not in the docstring, so this checker refuses to guess it: creatable,
        and requiredness is left to the human who reads the sentence."""
        self.assertEqual(self.sections(self.INVOICE)["due"], drift.SECTION_CREATE)

    def testSignatureRequiredIsTheParametersWithNoDefault(self):
        import ast as astModule
        function = astModule.parse(
            "def __init__(self, amount, taxId, fine=None):\n    pass\n").body[0]
        self.assertEqual(drift.signatureRequired(function), set(["amount", "taxId"]))

    def testAgreementIsSilent(self):
        findings = []
        drift.checkFlags(self.table([("amount", drift.FLAG_REQUIRED | drift.FLAG_CREATE),
                                     ("fine", drift.FLAG_CREATE),
                                     ("brcode", 0)]),
                         self.python(self.sections(self.INVOICE), ["amount", "taxId"]),
                         findings)
        self.assertEqual(findings, [])

    def testADroppedRequiredBitFires(self):
        findings = []
        drift.checkFlags(self.table([("amount", drift.FLAG_CREATE)]),
                         self.python(self.sections(self.INVOICE), ["amount", "taxId"]),
                         findings)
        self.assertEqual([item["key"] for item in findings], ["flag.conflict:Invoice:amount"])

    def testACreateFieldTurnedReadOnlyFires(self):
        findings = []
        drift.checkFlags(self.table([("fine", 0)]),
                         self.python(self.sections(self.INVOICE), ["amount", "taxId"]),
                         findings)
        self.assertEqual([item["key"] for item in findings], ["flag.conflict:Invoice:fine"])

    def testAReturnOnlyFieldMarkedCreatableFires(self):
        findings = []
        drift.checkFlags(self.table([("brcode", drift.FLAG_CREATE)]),
                         self.python(self.sections(self.INVOICE), ["amount", "taxId"]),
                         findings)
        self.assertEqual([item["key"] for item in findings], ["flag.conflict:Invoice:brcode"])

    def testAPositionalReturnOnlyAttributeIsNotRequiredness(self):
        """invoice.Log is __init__(self, id, created, type, errors, invoice):
        every attribute is positional because from_api_json builds it, and
        none of them is something a caller supplies."""
        sections = self.sections("""# invoice.Log object
    ## Attributes (return-only):
    - id [string]: unique id.
    - created [datetime]: creation datetime.
    """)
        findings = []
        drift.checkFlags(self.table([("id", 0), ("created", 0)], name="InvoiceLog"),
                         self.python(sections, ["id", "created"], name="InvoiceLog"),
                         findings)
        self.assertEqual(findings, [])

    def testPythonContradictingItselfFires(self):
        """Transfer.account_type: positional with no default, and documented
        as optional with a default. That is a finding in its own right."""
        findings = []
        drift.checkFlags(self.table([("fine", drift.FLAG_CREATE)]),
                         self.python(self.sections(self.INVOICE), ["fine"]),
                         findings)
        self.assertEqual([item["key"] for item in findings], ["flag.conflict:Invoice:fine"])
        self.assertIn("lists it as optional", findings[0]["detail"])

    def testAFieldTheTableHasAndPythonDoesNotIsLeftToFieldGone(self):
        findings = []
        drift.checkFlags(self.table([("displayDescription", drift.FLAG_CREATE)]),
                         self.python(self.sections(self.INVOICE), ["amount"]),
                         findings)
        self.assertEqual(findings, [])

    def testAFlagConflictCannotBeAccepted(self):
        conflicts = [drift.finding("flag.conflict", "Invoice", "taxId", "REQUIRED disagrees")]
        accepted = {"accepted": [{"key": "flag.conflict:Invoice:taxId", "owner": "me",
                                  "reason": "r"}]}
        status, report = drift.reconcile(conflicts, accepted)
        self.assertEqual(status, 1)
        self.assertIn("HARD STOP", report)

    def testAResolutionClearsAFlagConflict(self):
        conflicts = [drift.finding("flag.conflict", "Invoice", "taxId", "REQUIRED disagrees")]
        accepted = {"accepted": [{"key": "flag.conflict:Invoice:taxId", "owner": "me",
                                  "reason": "r", "resolution": "table"}]}
        self.assertEqual(drift.reconcile(conflicts, accepted)[0], 0)


class Endpoints(unittest.TestCase):

    def testDerivesThePathForEveryVerbShape(self):
        self.assertEqual(drift.verbEndpoint("POST_MULTI", "invoice", None), ("POST", "/v2/invoice"))
        self.assertEqual(drift.verbEndpoint("GET_ID", "invoice", None), ("GET", "/v2/invoice/:id"))
        self.assertEqual(drift.verbEndpoint("GET_FIRST", "balance", None), ("GET", "/v2/balance"))
        self.assertEqual(drift.verbEndpoint("QUERY", "invoice/log", None), ("GET", "/v2/invoice/log"))
        self.assertEqual(drift.verbEndpoint("PATCH_ID", "invoice", None), ("PATCH", "/v2/invoice/:id"))
        self.assertEqual(drift.verbEndpoint("DELETE_ID", "transfer", None), ("DELETE", "/v2/transfer/:id"))
        self.assertEqual(drift.verbEndpoint("CONTENT", "invoice", "pdf"), ("GET", "/v2/invoice/:id/pdf"))
        self.assertEqual(drift.verbEndpoint("SUB_RESOURCE", "invoice", "payment"),
                         ("GET", "/v2/invoice/:id/payment"))
        self.assertIsNone(drift.verbEndpoint("NEW", "invoice", None))


class Types(unittest.TestCase):

    def testDocsTagAndTableTypeMustAgree(self):
        self.assertTrue(drift.tagFitsType("INTEGER", "AMOUNT"))
        self.assertTrue(drift.tagFitsType("STRING", "DATE_OR_DATETIME"))
        self.assertFalse(drift.tagFitsType("STRING", "BOOL"))
        self.assertFalse(drift.tagFitsType("INTEGER", "STRING"))

    def testPythonCoercionAndTableTypeMustAgree(self):
        self.assertTrue(drift.checkFitsType("check_datetime_or_date", "DATE_OR_DATETIME"))
        self.assertFalse(drift.checkFitsType("check_datetime_or_date", "DATETIME"))
        self.assertTrue(drift.checkFitsType("check_timedelta", "SECONDS"))
        self.assertFalse(drift.checkFitsType(None, "SECONDS"))
        self.assertTrue(drift.checkFitsType(None, "STRING"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
