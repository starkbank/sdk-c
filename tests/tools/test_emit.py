"""Unit cases for tools/emit.py.

Everything this emits lives outside the ABI - bindings and docs samples - and
all of it is committed, so CI regenerates and diffs it. That makes two
properties load-bearing: the output is byte-deterministic, and the emitter
cannot write anywhere but the three places it owns.

Run: python3 tests/tools/test_emit.py
"""

import os
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import emit


TABLE = {
    "name": "Widget",
    "endpoint": "widget",
    "queryKeys": ["limit", "status"],
    "fields": [
        {"key": "amount", "type": 1, "flags": 7, "ref": None},
        {"key": "taxId", "type": 0, "flags": 3, "ref": None},
        {"key": "tags", "type": 9, "flags": 2, "ref": None},
        {"key": "status", "type": 0, "flags": 4, "ref": None},
        {"key": "id", "type": 0, "flags": 0, "ref": None},
    ],
}

# A REQUIRED RESOURCE field whose ref names a real family - type 12 is
# STARKBANK_FIELD_RESOURCE, type 4 is STARKBANK_FIELD_NUMBER, flags 3 is
# REQUIRED | CREATE. This is what tests/tools/test_emit.py did not have when
# the emitter shipped a PaymentRequest.create sample that could never
# succeed: every REQUIRED field in TABLE above is a scalar, so
# requiredSetters' `if setter is None: continue` had nothing to silently
# drop and the gap stayed green.
NESTED_TABLE = {
    "name": "Nested",
    "endpoint": "nested",
    "queryKeys": [],
    "fields": [{"key": "value", "type": 4, "flags": 3, "ref": None}],
}

RESOURCE_TABLE = {
    "name": "Box",
    "endpoint": "box",
    "queryKeys": [],
    "fields": [{"key": "content", "type": 12, "flags": 3, "ref": "Nested"}],
}

# The same REQUIRED RESOURCE type, but with no ref at all - a return-only
# polymorph like Event.log carries this shape; PaymentRequest.payment is the
# one REQUIRED example. With no ref and no override there is no family to
# construct, so the fallback is a literal JSON object.
POLYMORPH_TABLE = {
    "name": "Envelope",
    "endpoint": "envelope",
    "queryKeys": [],
    "fields": [{"key": "payload", "type": 12, "flags": 3, "ref": None}],
}

# type 11 is STARKBANK_FIELD_LIST_RESOURCE. "Missing" resolves to no table and
# no ident, so this field has no sample strategy at all - the case the
# emitter must now refuse to emit silently.
LIST_RESOURCE_TABLE = {
    "name": "Crate",
    "endpoint": "crate",
    "queryKeys": [],
    "fields": [{"key": "items", "type": 11, "flags": 3, "ref": "Missing"}],
}

PAYMENT_REQUEST_TABLE = {
    "name": "PaymentRequest",
    "endpoint": "payment-request",
    "queryKeys": [],
    "fields": [{"key": "payment", "type": 12, "flags": 3, "ref": None}],
}

TRANSFER_TABLE = {
    "name": "Transfer",
    "endpoint": "transfer",
    "queryKeys": [],
    "fields": [{"key": "amount", "type": 1, "flags": 3, "ref": None}],
}


class Samples(unittest.TestCase):

    def testOneFilePerMechanicalVerb(self):
        self.assertEqual(emit.sampleName("invoice_log", "query"), "invoice-log-query.c")

    def testACreateSampleSetsEveryRequiredFieldAndNothingElse(self):
        source = emit.sampleSource("widget", TABLE, "create", "POST_MULTI", None)
        self.assertIn("starkbank_entity_set_amount(widget, \"amount\", 400000)", source)
        self.assertIn("starkbank_entity_set_string(widget, \"taxId\"", source)
        self.assertNotIn("\"tags\"", source)        # CREATE without REQUIRED
        self.assertNotIn("\"status\"", source)      # PATCH only
        self.assertNotIn("set_string(widget, \"id\"", source)   # return-only

    def testARequiredResourceFieldWithAKnownRefBuildsAndDumpsIt(self):
        """The documented dance for a REQUIRED RESOURCE/OBJECT field whose ref
        names a real family: build it with its own constructor, fill in its
        own required fields, dump it, and embed the JSON."""
        tables = {"Nested": NESTED_TABLE}
        identForName = {"Nested": "nested"}
        source = emit.sampleSource("box", RESOURCE_TABLE, "create", "POST_MULTI", None,
                                   None, tables, identForName)
        self.assertIn("starkbank_nested_new(&content)", source)
        self.assertIn("starkbank_entity_set_number(content, \"value\", 5)", source)
        self.assertIn("starkbank_entity_dump(content, &contentJson, NULL)", source)
        self.assertIn("starkbank_entity_set_json_raw(box, \"content\", contentJson)", source)
        self.assertIn("starkbank_entity_free(content)", source)

    def testARequiredResourceFieldWithNoRefGetsALiteralJsonObject(self):
        """The gap the finding named: every REQUIRED field in TABLE is a
        scalar, so the old requiredSetters' `if setter is None: continue` had
        nothing to silently drop and stayed green. A polymorphic REQUIRED
        RESOURCE field (no ref, no override) must still get a working sample
        strategy instead of vanishing."""
        source = emit.sampleSource("envelope", POLYMORPH_TABLE, "create", "POST_MULTI", None)
        self.assertIn("starkbank_entity_set_json_raw(envelope, \"payload\", \"{}\")", source)

    def testARequiredFieldWithNoSampleStrategyFailsLoudly(self):
        """requiredSetters used to skip a REQUIRED field it could not fill in
        - exactly how PaymentRequest.create shipped a sample that omitted the
        one field the API requires. It must refuse to emit rather than emit
        something broken."""
        with self.assertRaises(ValueError):
            emit.sampleSource("crate", LIST_RESOURCE_TABLE, "create", "POST_MULTI", None)

    def testPaymentRequestPaymentAlsoSetsTypeSoThePolymorphResolves(self):
        """PaymentRequest.payment has no ref - it hydrates as whichever family
        the sibling "type" field names - so a runnable sample has to pick one
        and say so on both sides or the create fails validation."""
        tables = {"Transfer": TRANSFER_TABLE}
        identForName = {"Transfer": "transfer"}
        source = emit.sampleSource("payment_request", PAYMENT_REQUEST_TABLE, "create",
                                   "POST_MULTI", None, None, tables, identForName)
        self.assertIn("starkbank_transfer_new(&payment)", source)
        self.assertIn("starkbank_entity_set_string(payment_request, \"type\", \"transfer\")", source)

    def testAPostSingleSampleHandsOverAnEntityAndFreesBoth(self):
        """The shape differs from POST_MULTI in the one way that matters to a
        caller: no list, and the entity stays theirs to free."""
        source = emit.sampleSource("widget", TABLE, "create", "POST_SINGLE", None)
        self.assertIn("starkbank_widget_create(client, widget, &created, &errors)", source)
        self.assertNotIn("starkbank_list_", source)
        self.assertIn("starkbank_entity_free(widget)", source)
        self.assertIn("starkbank_entity_free(created)", source)

    def testAListOfVocabularyValuesGetsAValueFromThatVocabulary(self):
        """A Webhook sample that subscribes to "war" is a sample nobody pastes."""
        setter, value = emit.sampleValue({"key": "subscriptions", "type": 9, "flags": 3})
        self.assertEqual((setter, value), ("append_string", "\"invoice\""))

    def testEverySampleFreesWhatItAllocates(self):
        for verb, shape in (("create", "POST_MULTI"), ("create", "POST_SINGLE"),
                            ("get", "GET_ID"), ("query", "QUERY"),
                            ("page", "PAGE"), ("update", "PATCH_ID"), ("delete", "DELETE_ID"),
                            ("pdf", "CONTENT"), ("qrcode", "CONTENT_INT"),
                            ("payment", "SUB_RESOURCE"), ("get", "GET_FIRST")):
            source = emit.sampleSource("widget", TABLE, verb, shape, None)
            self.assertIn("starkbank_client_free(client)", source)
            self.assertIn("int main(void)", source)
            self.assertTrue(source.endswith("}\n"), verb)
            if "starkbank_list *" in source:
                self.assertIn("starkbank_list_free", source)
            if "starkbank_iter *" in source:
                self.assertIn("starkbank_iter_free", source)

    def testTheEndpointIsInTheHeaderComment(self):
        source = emit.sampleSource("widget", TABLE, "get", "GET_ID", None)
        self.assertIn("GET /v2/widget/:id", source)

    def testTheBoilerplateIsMarkedForElisionByTheDocsSite(self):
        source = emit.sampleSource("widget", TABLE, "get", "GET_ID", None)
        self.assertIn("/*<*/", source)
        self.assertIn("/*>*/", source)

    def testAVerbWithNoSampleShapeIsSkipped(self):
        self.assertIsNone(emit.sampleSource("widget", TABLE, "parse", "HANDWRITTEN", None))
        self.assertIsNone(emit.sampleSource("widget", TABLE, "new", "NEW", None))


class WriteGuard(unittest.TestCase):
    """Only owned() is exercised against real repo paths.

    Calling writeOrCheck on an in-repo path to prove it refuses is how a red
    test destroys a source file: the assertion only runs after the call, and a
    missing guard means the call already wrote. The predicate is pure, so it is
    the thing to test against real paths; writeOrCheck is tested against a
    scratch file that no build depends on.
    """

    def testTheEmitterRefusesToWriteOutsideWhatItOwns(self):
        for path in ("starkc/entity.c", "include/starkbank.h", "Makefile",
                     "../elsewhere.c", "tests/run.c"):
            self.assertFalse(emit.owned(os.path.join(ROOT, path)), path)

    def testWriteOrCheckEnforcesThePredicate(self):
        with tempfile.TemporaryDirectory() as directory:
            target = os.path.join(directory, "not-ours.c")
            with self.assertRaises(ValueError):
                emit.writeOrCheck(target, "x", False)
            self.assertFalse(os.path.exists(target))

    def testTheThreeItOwnsAreAllowed(self):
        for path in ("bindings/Starkbank.cs", "samples/invoice-get.c",
                     "include/starkbank_fields.h"):
            self.assertTrue(emit.owned(os.path.join(ROOT, path)), path)


class Determinism(unittest.TestCase):

    def testNoGeneratorVersionLeaksIntoAnEmittedFile(self):
        """The version lives in .genstamp alone: a generator bump must never
        rewrite the tree and bury the real diff."""
        source = emit.sampleSource("widget", TABLE, "get", "GET_ID", None)
        self.assertNotIn(emit.GENSTAMP, source)

    def testTwoRunsAgree(self):
        first = emit.sampleSource("widget", TABLE, "query", "QUERY", None)
        second = emit.sampleSource("widget", TABLE, "query", "QUERY", None)
        self.assertEqual(first, second)


if __name__ == "__main__":
    unittest.main(verbosity=2)
