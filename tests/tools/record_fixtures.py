#!/usr/bin/env python3
"""Record the slice's goldens from sdk-python, which is normative.

Every expected byte in tests/reference/slice.json came out of the reference
implementation running against a stub transport. Nothing here reaches the
network: `requests` is replaced in sys.modules before starkbank is imported,
so an accidental real call is an ImportError-shaped failure rather than a
packet. No pip install and no writes outside tests/reference/.

Two things are recorded per case:

  request    the method, url, query and body python emitted
  hydrated   json.dumps(api_json(obj), sort_keys=True) for each object python
             built out of a canned response

Bodies and query strings are stored raw AND normalised. The C tier signs the
bytes it sends and python signs the bytes it sends, so the two need not agree
on key order or on json.dumps' ", " separators - they must agree on content.
Normalising is therefore sorted keys and the compact separators on both sides,
and the raw form is kept beside it so a reviewer can see what python actually
put on the wire.
"""

import json
import os
import sys
import types
import subprocess
from datetime import datetime, date


HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
OUT = os.path.join(ROOT, "tests", "reference", "slice.json")
PINNED_SHA = os.path.join(ROOT, "tests", "reference", "sdk-python.sha")
USAGE = "usage: record_fixtures.py [--python=DIR] [--core=DIR] [--ecdsa=DIR] [--unpinned]\n"

# The sibling layout the Makefile's CORE_PREFIX/ECDSA_PREFIX loops already
# assume. Relative, because an absolute path to somebody's home directory in a
# repo that ships publicly is a bug in itself - and because this recorder is
# what makes the goldens recorded rather than believed, so which tree it reads
# has to be visible and overridable.
DEFAULT_PATHS = {
    "core": os.path.join(ROOT, os.pardir, os.pardir, "starkinfra", "core-python"),
    "ecdsa": os.path.join(ROOT, os.pardir, "ecdsa-python"),
    "python": os.path.join(ROOT, os.pardir, "sdk-python"),
}


def parseArguments(argv):
    paths = dict(DEFAULT_PATHS)
    unpinned = False
    for argument in argv:
        name = argument.split("=", 1)[0][2:] if argument.startswith("--") else ""
        if name in paths and "=" in argument:
            paths[name] = argument.split("=", 1)[1]
            continue
        if argument == "--unpinned":
            unpinned = True
            continue
        sys.stderr.write(USAGE)
        raise SystemExit(2)
    return paths, unpinned


def observedSha(root):
    try:
        output = subprocess.check_output(["git", "-C", root, "rev-parse", "HEAD"],
                                         stderr=subprocess.DEVNULL)
    except (OSError, subprocess.CalledProcessError):
        return None
    return output.decode("utf-8").strip()


def readPin():
    with open(PINNED_SHA) as handle:
        for line in handle:
            line = line.split("#", 1)[0].strip()
            if line:
                return line
    raise SystemExit("record_fixtures: no sha in %s" % PINNED_SHA)


def requirePin(root, unpinned):
    """Refuse to record from an unpinned tree, mirroring check-drift-pinned.

    tests/reference/sdk-python.sha exists so the recording is reproducible.
    Until now the pin was enforced only by drift.py, so the recorder could and
    did record from whatever working tree happened to sit at the path - which
    makes "recorded from sdk-python" a claim about a machine, not a commit.
    """
    if unpinned:
        sys.stderr.write("record_fixtures: --unpinned, recording from %s as it stands\n" % root)
        return
    pinned = readPin()
    observed = observedSha(root)
    if observed == pinned:
        return
    sys.stderr.write(
        "record_fixtures: %s is at %s and tests/reference/sdk-python.sha pins %s\n"
        % (root, observed[:12] if observed else "an unknown commit", pinned[:12])
        + "record_fixtures: refusing to write; check out the pin or pass --unpinned\n")
    raise SystemExit(1)


PATHS, UNPINNED = parseArguments(sys.argv[1:])


class StubResponse:
    def __init__(self, status, content):
        self.status_code = status
        self.content = content
        self.headers = {}


class Stub:
    """The whole transport. Records one call, replies with what was queued."""

    def __init__(self):
        self.calls = []
        self.queue = []

    def reply(self, *bodies):
        self.queue = [self._encode(body) for body in bodies]

    @staticmethod
    def _encode(body):
        if isinstance(body, bytes):
            return (200, body)
        if isinstance(body, tuple):
            status, content = body
            return (status, content if isinstance(content, bytes)
                    else json.dumps(content).encode("utf-8"))
        return (200, json.dumps(body).encode("utf-8"))

    def method(self, name):
        def call(url=None, data=None, headers=None, timeout=None):
            self.calls.append({"method": name, "url": url, "body": data})
            if not self.queue:
                raise AssertionError("stub ran out of queued replies for " + str(url))
            status, content = self.queue.pop(0)
            return StubResponse(status, content)
        return call


STUB = Stub()


def install():
    module = types.ModuleType("requests")
    for name in ("get", "post", "delete", "patch", "put"):
        setattr(module, name, STUB.method(name.upper()))
    sys.modules["requests"] = module
    # Appended, never inserted at 0: PYTHONPATH is how a caller points this at
    # a different checkout, and a default that wins over it is not a default.
    for name in ("core", "ecdsa", "python"):
        path = os.path.abspath(PATHS[name])
        if path not in sys.path:
            sys.path.append(path)


requirePin(PATHS["python"], UNPINNED)
install()

import starkbank                                             # noqa: E402
from starkcore.utils.api import api_json                     # noqa: E402


def canonical(value):
    """Sorted keys, compact separators, and 10.0 spelled 10.

    The last of those is not cosmetic. starkcore's DOM has one number type and
    prints an integral value as an integer, where python's json keeps the
    float it was handed. Both are the number ten and both are accepted, so
    canonicalising here compares content instead of failing on a distinction
    C cannot make. Everything else - encoding, ordering within a list, string
    escapes - is compared exactly.
    """
    return json.dumps(_canonicalNumbers(value), sort_keys=True, separators=(",", ":"),
                      default=_stringify)


def _canonicalNumbers(value):
    if isinstance(value, bool):
        return value
    if isinstance(value, float) and value == int(value):
        return int(value)
    if isinstance(value, dict):
        return {k: _canonicalNumbers(v) for k, v in value.items()}
    if isinstance(value, list):
        return [_canonicalNumbers(v) for v in value]
    return value


def normalise_body(raw):
    if not raw:
        return ""
    return canonical(json.loads(raw))


def split_url(url):
    if "?" not in url:
        return url, ""
    path, query = url.split("?", 1)
    return path, query


def normalise_query(query):
    """Sorted key=value pairs, encoding untouched: %2C vs , is a real diff."""
    if not query:
        return ""
    return "&".join(sorted(query.split("&")))


def hydrated(value):
    if isinstance(value, list):
        return [hydrated(item) for item in value]
    return canonical(api_json(value))


def _stringify(value):
    if isinstance(value, datetime):
        return value.strftime("%Y-%m-%dT%H:%M:%S+00:00")
    if isinstance(value, date):
        return value.strftime("%Y-%m-%d")
    raise TypeError(repr(value))


CASES = {}


def record(name, replies, call):
    STUB.calls = []
    STUB.reply(*replies)
    result = call()
    if hasattr(result, "__next__") or isinstance(result, type(iter([]))):
        result = list(result)
    requests = []
    for entry in STUB.calls:
        path, query = split_url(entry["url"])
        requests.append({
            "method": entry["method"],
            "url": path,
            "query": normalise_query(query),
            "queryRaw": query,
            "body": normalise_body(entry["body"]),
            "bodyRaw": entry["body"] or "",
        })
    case = {"requests": requests}
    if result is not None:
        case["hydrated"] = hydrated(result)
    CASES[name] = case
    return result


# ------------------------------------------------------------------ bodies
#
# Canned responses, written here rather than fetched: they are inputs to the
# recorder, not outputs of it. `fee: null` is deliberate - python drops a null
# on the way out of api_json and the C tier must agree that a null is an
# absence, not a value.

INVOICE = {
    "id": "5155165527080960",
    "amount": 0,
    "taxId": "20.018.183/0001-80",
    "name": "Iron Bank S.A.",
    "due": "2026-10-28",
    "expiration": 123456789,
    "fine": 2.5,
    "interest": 1.3,
    "discounts": [{"percentage": 10.0, "due": "2026-10-01"}],
    "descriptions": [{"key": "service", "value": "swords"}],
    "rules": [{"key": "allowedTaxIds", "value": ["012.345.678-90", "45.059.493/0001-73"]}],
    "splits": [{"id": "6", "amount": 141, "receiverId": "5706627130851328",
                "status": "created", "created": "2026-09-16T12:00:00+00:00"}],
    "tags": ["war", "supply"],
    "status": "registered",
    "pdf": "https://invoice.starkbank.com/pdf/d454fa4e",
    "link": "https://my-workspace.sandbox.starkbank.com/invoicelink/d454fa4e",
    "brcode": "00020101021226800014br.gov.bcb.pix",
    "nominalAmount": 400000,
    "fineAmount": 20000,
    "interestAmount": 10000,
    "discountAmount": 3000,
    "fee": None,
    "transactionIds": ["19827356981273"],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

INVOICE_PAYMENT = {
    "amount": 1234,
    "name": "Anthony Edward Stark",
    "taxId": "20.018.183/0001-80",
    "bankCode": "20018183",
    "branchCode": "1357-9",
    "accountNumber": "876543-2",
    "accountType": "checking",
    "endToEndId": "E79457883202101262140HHX553UPqeq",
    "method": "pix",
}

INVOICE_LOG = {
    "id": "6341320293482496",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "paid",
    "errors": [],
    "invoice": INVOICE,
}

TRANSFER = {
    "id": "5155165527080960",
    "amount": 1000,
    "name": "Daenerys Targaryen Stormborn",
    "taxId": "594.739.480-42",
    "bankCode": "20018183",
    "branchCode": "1357-9",
    "accountNumber": "876543-2",
    "accountType": "checking",
    "externalId": "my-internal-id-123456",
    "scheduled": "2026-10-28T17:59:26+00:00",
    "description": "Payment for service #1234",
    "displayDescription": "Sword sharpening",
    "tags": ["daenerys", "invoice/1234"],
    "rules": [{"key": "resendingLimit", "value": 5}],
    "fee": 200,
    "status": "processing",
    "transactionIds": ["19827356981273"],
    "metadata": {"tracker": "abc", "attempt": 2},
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

TRANSFER_LOG = {
    "id": "6341320293482496",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "transfer": TRANSFER,
}

EVENT = {
    "id": "5764898044149760",
    "created": "2026-09-16T12:00:00+00:00",
    "isDelivered": False,
    "subscription": "invoice",
    "workspaceId": "4545454545454545",
    "log": INVOICE_LOG,
}

EVENT_TRANSFER = dict(EVENT, id="4823", subscription="transfer", log=TRANSFER_LOG)
EVENT_UNKNOWN = dict(EVENT, id="4824", subscription="pix-request",
                     log={"id": "1", "created": "2026-09-16T12:00:00+00:00"})

EVENT_ATTEMPT = {
    "id": "1616161616161616",
    "code": "500",
    "message": "Internal Server Error",
    "eventId": "5764898044149760",
    "webhookId": "6178044066660352",
    "created": "2026-09-16T12:00:00+00:00",
}

WEBHOOK = {
    "id": "6178044066660352",
    "url": "https://webhook.site/60e9c18e-4b5c-4369-bda1-ab5fcd8e1b29",
    "subscriptions": ["transfer", "invoice"],
}

# The four previews sdk-python's _sub_resource_by_type names, and nothing else:
# a fifth type is the "this build predates it" case PAYMENT_PREVIEW_UNKNOWN
# covers. Zeroes are deliberate here too - a discount of 0 is a real answer.
BRCODE_PREVIEW = {
    "status": "created",
    "name": "Tony Stark",
    "taxId": "012.345.678-90",
    "bankCode": "20018183",
    "accountType": "checking",
    "allowChange": True,
    "amount": 1000,
    "nominalAmount": 900,
    "interestAmount": 50,
    "fineAmount": 50,
    "reductionAmount": 0,
    "discountAmount": 0,
    "reconciliationId": "tx-123",
    "description": "Payment for service #1234",
}

BOLETO_PREVIEW = {
    "status": "active",
    "amount": 23456,
    "discountAmount": 0,
    "fineAmount": 0,
    "interestAmount": 0,
    "due": "2026-10-28",
    "expiration": "2026-11-27",
    "name": "Anthony Edward Stark",
    "taxId": "20.018.183/0001-80",
    "receiverName": "Iron Bank S.A.",
    "receiverTaxId": "20.018.183/0001-80",
    "payerName": "Arya Stark",
    "payerTaxId": "012.345.678-90",
    "line": "34191.09008 63571.277308 71444.640008 5 81960000000062",
    "barCode": "34195819600000000621090063571277307144464000",
}

TAX_PREVIEW = {
    "amount": 23456,
    "name": "Iron Throne",
    "description": "ISS Payment - Iron Throne",
    "line": "85660000006 6 67940064007 5 41190025511 7 00010601813 8",
    "barCode": "85660000006679400640074119002551100010601813",
}

UTILITY_PREVIEW = {
    "amount": 23456,
    "name": "Light Company",
    "description": "Utility Payment - Light Company",
    "line": "82660000002 8 44361143007 7 41190025511 7 00010601813 8",
    "barCode": "82660000002443611430074119002551100010601813",
}

BRCODE = ("00020126580014br.gov.bcb.pix0136a629532e-7693-4846-852d-1bbff817b5a8"
          "520400005303986540510.005802BR5908T'Challa6009Sao Paulo62090505123456304B14A")
BOLETO_LINE = "34191.09008 63571.277308 71444.640008 5 81960000000062"
TAX_LINE = "85660000006 6 67940064007 5 41190025511 7 00010601813 8"
UTILITY_LINE = "82660000002 8 44361143007 7 41190025511 7 00010601813 8"

PAYMENT_PREVIEW_BRCODE = {
    "id": BRCODE, "scheduled": "2026-10-28",
    "type": "brcode-payment", "payment": BRCODE_PREVIEW,
}
PAYMENT_PREVIEW_BOLETO = {
    "id": BOLETO_LINE, "scheduled": "2026-10-28",
    "type": "boleto-payment", "payment": BOLETO_PREVIEW,
}
PAYMENT_PREVIEW_TAX = {
    "id": TAX_LINE, "scheduled": "2026-10-28",
    "type": "tax-payment", "payment": TAX_PREVIEW,
}
PAYMENT_PREVIEW_UTILITY = {
    "id": UTILITY_LINE, "scheduled": "2026-10-28",
    "type": "utility-payment", "payment": UTILITY_PREVIEW,
}
# A type this build predates: python leaves payment a plain dict and sdk-c
# leaves it untagged and permissive. Same path, both tiers.
PAYMENT_PREVIEW_UNKNOWN = {
    "id": "5656565656565656", "scheduled": "2026-10-28",
    "type": "pix-reversal", "payment": {"id": "1", "amount": 42},
}

BOLETO = {
    "id": "5155165527080960",
    "amount": 23456,
    "name": "Anthony Edward Stark",
    "taxId": "012.345.678-90",
    "streetLine1": "Av. Paulista, 200",
    "streetLine2": "Apto. 123",
    "district": "Bela Vista",
    "city": "Sao Paulo",
    "stateCode": "SP",
    "zipCode": "01311-200",
    "due": "2026-10-28",
    "fine": 2.5,
    "interest": 1.0,
    "overdueLimit": 59,
    "descriptions": [{"text": "sword sharpening", "amount": 1234}],
    "discounts": [{"percentage": 10.0, "date": "2026-10-01"}],
    "tags": ["war", "supply"],
    "receiverName": "Iron Bank S.A.",
    "receiverTaxId": "20.018.183/0001-80",
    "fee": 200,
    "line": "34191.09008 63571.277308 71444.640008 5 81960000000062",
    "barCode": "34195819600000000621090063571277307144464000",
    "status": "registered",
    "transactionIds": ["19827356981273"],
    "workspaceId": "4545454545454545",
    "ourNumber": "10131474",
    "created": "2026-09-16T12:00:00+00:00",
}

BOLETO_LOG = {
    "id": "6341320293482496",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "registered",
    "errors": [],
    "boleto": BOLETO,
}

BOLETO_PAYMENT = {
    "id": "5155165527080960",
    "taxId": "20.018.183/0001-80",
    "description": "sword sharpening",
    "line": "34191.09008 63571.277308 71444.640008 5 81960000000062",
    "barCode": "34195819600000000621090063571277307144464000",
    "amount": 23456,
    "scheduled": "2026-10-28",
    "tags": ["war", "supply"],
    "status": "success",
    "fee": 200,
    "transactionIds": ["19827356981273"],
    "created": "2026-09-16T12:00:00+00:00",
}

BOLETO_PAYMENT_LOG = {
    "id": "6341320293482497",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "payment": BOLETO_PAYMENT,
}

BRCODE_PAYMENT = {
    "id": "5155165527080960",
    "brcode": BRCODE,
    "taxId": "012.345.678-90",
    "description": "sword sharpening",
    "amount": 23456,
    "scheduled": "2026-10-28",
    "tags": ["war", "supply"],
    "rules": [{"key": "resendingLimit", "value": 5}],
    "name": "Tony Stark",
    "status": "success",
    "type": "dynamic",
    "transactionIds": ["19827356981273"],
    "fee": 50,
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

BRCODE_PAYMENT_LOG = {
    "id": "6341320293482498",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "payment": BRCODE_PAYMENT,
}

BALANCE = {
    "id": "5155165527080960",
    "amount": 1234567,
    "currency": "BRL",
    "updated": "2026-09-16T12:00:00+00:00",
}

# line and barCode are the conditionally-required pair: both are sent below,
# and both come back on the reply, exactly as boleto payment's line/barCode
# do. scheduled is a plain date, not a datetime.
UTILITY_PAYMENT = {
    "id": "5155165527080960",
    "line": "82660000002 8 44361143007 7 41190025511 7 00010601813 8",
    "barCode": "82660000002443611430074119002551100010601813",
    "description": "Utility Payment - Light Company",
    "scheduled": "2026-09-20",
    "tags": ["electricity", "light"],
    "status": "success",
    "amount": 23456,
    "fee": 200,
    "type": "utility",
    "transactionIds": ["19827356981273"],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

UTILITY_PAYMENT_LOG = {
    "id": "6341320293482496",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "payment": UTILITY_PAYMENT,
}

TAX_PAYMENT = {
    "id": "5155165527080961",
    "line": "85660000006 6 67940064007 5 41190025511 7 00010601813 8",
    "barCode": "85660000006679400640074119002551100010601813",
    "description": "ISS Payment - Iron Throne",
    "scheduled": "2026-09-21",
    "tags": ["iss", "throne"],
    "type": "iss",
    "status": "success",
    "amount": 23456,
    "fee": 150,
    "transactionIds": ["19827356981274"],
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

TAX_PAYMENT_LOG = {
    "id": "6341320293482497",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "payment": TAX_PAYMENT,
}

# Fully structured: no line/barCode pair, and every required field is
# present - the contrast case to UtilityPayment/TaxPayment's conditional pair.
DARF_PAYMENT = {
    "id": "5155165527080962",
    "revenueCode": "5948",
    "taxId": "20.018.183/0001-80",
    "competence": "2026-08-31",
    "referenceNumber": "08.1.17.00-4",
    "fineAmount": 234,
    "interestAmount": 456,
    "due": "2026-10-17",
    "description": "DARF Payment - competence August",
    "tags": ["darf", "federal"],
    "scheduled": "2026-09-22",
    "status": "success",
    "amount": 24146,
    "nominalAmount": 23456,
    "fee": 0,
    "transactionIds": ["19827356981275"],
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

DARF_PAYMENT_LOG = {
    "id": "6341320293482498",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "payment": DARF_PAYMENT,
}

PDF = b"%PDF-1.4 fake"
PNG = b"\x89PNG\r\n\x1a\n fake"


def main():
    key = open(os.path.join(ROOT, "tests", "fixtures", "privateKey.pem")).read()
    user = starkbank.Project(environment="sandbox", id="5656565656565656", private_key=key)
    starkbank.user = user

    invoice = starkbank.Invoice(
        amount=400000,
        tax_id="012.345.678-90",
        name="Iron Bank S.A.",
        due="2026-10-28",
        expiration=123456789,
        fine=2.5,
        interest=1.3,
        tags=["war", "supply"],
        descriptions=[{"key": "service", "value": "swords"}],
        discounts=[{"percentage": 10.0, "due": "2026-10-01"}],
        rules=[starkbank.invoice.Rule(key="allowedTaxIds",
                                      value=["012.345.678-90", "45.059.493/0001-73"])],
        splits=[starkbank.Split(amount=141, receiver_id="5706627130851328")],
    )

    record("invoice.create", [{"invoices": [INVOICE]}],
           lambda: starkbank.invoice.create([invoice]))
    record("invoice.get", [{"invoice": INVOICE}],
           lambda: starkbank.invoice.get("5155165527080960"))
    record("invoice.query", [{"invoices": [INVOICE], "cursor": "c1"},
                             {"invoices": [INVOICE], "cursor": ""}],
           lambda: list(starkbank.invoice.query(limit=150, status="paid",
                                                tags=["war", "supply"],
                                                after="2026-01-01", before="2026-12-31")))
    record("invoice.page", [{"invoices": [INVOICE], "cursor": "c2"}],
           lambda: starkbank.invoice.page(cursor="c1", limit=10, status="paid")[0])
    record("invoice.update", [{"invoice": INVOICE}],
           lambda: starkbank.invoice.update("5155165527080960", status="canceled",
                                            amount=123, due="2026-11-01T12:00:00+00:00",
                                            expiration=7200))
    record("invoice.pdf", [PDF], lambda: None if starkbank.invoice.pdf("5155165527080960") else None)
    record("invoice.qrcode", [PNG],
           lambda: None if starkbank.invoice.qrcode("5155165527080960", size=12) else None)
    record("invoice.payment", [{"payment": INVOICE_PAYMENT}],
           lambda: starkbank.invoice.payment("5155165527080960"))
    record("invoice.log.get", [{"log": INVOICE_LOG}],
           lambda: starkbank.invoice.log.get("6341320293482496"))
    record("invoice.log.query", [{"logs": [INVOICE_LOG], "cursor": ""}],
           lambda: list(starkbank.invoice.log.query(limit=5, types=["paid"],
                                                    invoice_ids=["5155165527080960"])))
    record("invoice.log.page", [{"logs": [INVOICE_LOG], "cursor": ""}],
           lambda: starkbank.invoice.log.page(limit=5)[0])
    record("invoice.log.pdf", [PDF],
           lambda: None if starkbank.invoice.log.pdf("6341320293482496") else None)

    transfer = starkbank.Transfer(
        amount=1000,
        name="Daenerys Targaryen Stormborn",
        tax_id="594.739.480-42",
        bank_code="20018183",
        branch_code="1357-9",
        account_number="876543-2",
        account_type="checking",
        external_id="my-internal-id-123456",
        scheduled="2026-10-28T17:59:26+00:00",
        description="Payment for service #1234",
        display_description="Sword sharpening",
        tags=["daenerys", "invoice/1234"],
        rules=[starkbank.transfer.Rule(key="resendingLimit", value=5)],
    )

    record("transfer.create", [{"transfers": [TRANSFER]}],
           lambda: starkbank.transfer.create([transfer]))
    record("transfer.get", [{"transfer": TRANSFER}],
           lambda: starkbank.transfer.get("5155165527080960"))
    record("transfer.delete", [{"transfer": TRANSFER}],
           lambda: starkbank.transfer.delete("5155165527080960"))
    record("transfer.query", [{"transfers": [TRANSFER], "cursor": ""}],
           lambda: list(starkbank.transfer.query(limit=5, status="processing",
                                                 tax_id="594.739.480-42", sort="-created")))
    record("transfer.page", [{"transfers": [TRANSFER], "cursor": ""}],
           lambda: starkbank.transfer.page(limit=5)[0])
    record("transfer.pdf", [PDF],
           lambda: None if starkbank.transfer.pdf("5155165527080960") else None)
    record("transfer.log.get", [{"log": TRANSFER_LOG}],
           lambda: starkbank.transfer.log.get("6341320293482496"))
    record("transfer.log.query", [{"logs": [TRANSFER_LOG], "cursor": ""}],
           lambda: list(starkbank.transfer.log.query(limit=5, types=["success"],
                                                     transfer_ids=["5155165527080960"])))

    boleto = starkbank.Boleto(
        amount=23456,
        name="Anthony Edward Stark",
        tax_id="012.345.678-90",
        street_line_1="Av. Paulista, 200",
        street_line_2="Apto. 123",
        district="Bela Vista",
        city="Sao Paulo",
        state_code="SP",
        zip_code="01311-200",
        due="2026-10-28",
        fine=2.5,
        interest=1.0,
        overdue_limit=59,
        tags=["war", "supply"],
        descriptions=[{"text": "sword sharpening", "amount": 1234}],
        discounts=[{"percentage": 10.0, "date": "2026-10-01"}],
        receiver_name="Iron Bank S.A.",
        receiver_tax_id="20.018.183/0001-80",
    )

    record("boleto.create", [{"boletos": [BOLETO]}], lambda: starkbank.boleto.create([boleto]))
    record("boleto.get", [{"boleto": BOLETO}],
           lambda: starkbank.boleto.get("5155165527080960"))
    record("boleto.delete", [{"boleto": BOLETO}],
           lambda: starkbank.boleto.delete("5155165527080960"))
    record("boleto.query", [{"boletos": [BOLETO], "cursor": ""}],
           lambda: list(starkbank.boleto.query(limit=5, status="registered",
                                               tags=["war", "supply"])))
    record("boleto.page", [{"boletos": [BOLETO], "cursor": ""}],
           lambda: starkbank.boleto.page(limit=5)[0])
    record("boleto.pdf", [PDF],
           lambda: None if starkbank.boleto.pdf(
               "5155165527080960", layout="booklet",
               hidden_fields=["customerAddress", "customerTaxId"]) else None)
    record("boleto.log.get", [{"log": BOLETO_LOG}],
           lambda: starkbank.boleto.log.get("6341320293482496"))
    record("boleto.log.query", [{"logs": [BOLETO_LOG], "cursor": ""}],
           lambda: list(starkbank.boleto.log.query(limit=5, types=["registered"],
                                                   boleto_ids=["5155165527080960"])))
    record("boleto.log.page", [{"logs": [BOLETO_LOG], "cursor": ""}],
           lambda: starkbank.boleto.log.page(limit=5)[0])

    boleto_payment = starkbank.BoletoPayment(
        tax_id="20.018.183/0001-80",
        description="sword sharpening",
        line="34191.09008 63571.277308 71444.640008 5 81960000000062",
        scheduled="2026-10-28",
        tags=["war", "supply"],
    )

    record("boletopayment.create", [{"payments": [BOLETO_PAYMENT]}],
           lambda: starkbank.boletopayment.create([boleto_payment]))
    record("boletopayment.get", [{"payment": BOLETO_PAYMENT}],
           lambda: starkbank.boletopayment.get("5155165527080960"))
    record("boletopayment.delete", [{"payment": BOLETO_PAYMENT}],
           lambda: starkbank.boletopayment.delete("5155165527080960"))
    record("boletopayment.query", [{"payments": [BOLETO_PAYMENT], "cursor": ""}],
           lambda: list(starkbank.boletopayment.query(limit=5, status="success",
                                                       tags=["war", "supply"])))
    record("boletopayment.page", [{"payments": [BOLETO_PAYMENT], "cursor": ""}],
           lambda: starkbank.boletopayment.page(limit=5)[0])
    record("boletopayment.pdf", [PDF],
           lambda: None if starkbank.boletopayment.pdf("5155165527080960") else None)
    record("boletopayment.log.get", [{"log": BOLETO_PAYMENT_LOG}],
           lambda: starkbank.boletopayment.log.get("6341320293482497"))
    record("boletopayment.log.query", [{"logs": [BOLETO_PAYMENT_LOG], "cursor": ""}],
           lambda: list(starkbank.boletopayment.log.query(limit=5, types=["success"],
                                                          payment_ids=["5155165527080960"])))
    record("boletopayment.log.page", [{"logs": [BOLETO_PAYMENT_LOG], "cursor": ""}],
           lambda: starkbank.boletopayment.log.page(limit=5)[0])

    brcode_payment = starkbank.BrcodePayment(
        brcode=BRCODE,
        tax_id="012.345.678-90",
        description="sword sharpening",
        amount=23456,
        scheduled="2026-10-28",
        tags=["war", "supply"],
        rules=[starkbank.brcodepayment.Rule(key="resendingLimit", value=5)],
    )

    record("brcodepayment.create", [{"payments": [BRCODE_PAYMENT]}],
           lambda: starkbank.brcodepayment.create([brcode_payment]))
    record("brcodepayment.get", [{"payment": BRCODE_PAYMENT}],
           lambda: starkbank.brcodepayment.get("5155165527080960"))
    record("brcodepayment.query", [{"payments": [BRCODE_PAYMENT], "cursor": ""}],
           lambda: list(starkbank.brcodepayment.query(limit=5, status="success",
                                                       tags=["war", "supply"])))
    record("brcodepayment.page", [{"payments": [BRCODE_PAYMENT], "cursor": ""}],
           lambda: starkbank.brcodepayment.page(limit=5)[0])
    record("brcodepayment.update", [{"payment": BRCODE_PAYMENT}],
           lambda: starkbank.brcodepayment.update("5155165527080960", status="canceled"))
    record("brcodepayment.pdf", [PDF],
           lambda: None if starkbank.brcodepayment.pdf("5155165527080960") else None)
    record("brcodepayment.log.get", [{"log": BRCODE_PAYMENT_LOG}],
           lambda: starkbank.brcodepayment.log.get("6341320293482498"))
    record("brcodepayment.log.query", [{"logs": [BRCODE_PAYMENT_LOG], "cursor": ""}],
           lambda: list(starkbank.brcodepayment.log.query(limit=5, types=["success"],
                                                          payment_ids=["5155165527080960"])))
    record("brcodepayment.log.page", [{"logs": [BRCODE_PAYMENT_LOG], "cursor": ""}],
           lambda: starkbank.brcodepayment.log.page(limit=5)[0])

    record("event.get", [{"event": EVENT}], lambda: starkbank.event.get("5764898044149760"))
    record("event.query", [{"events": [EVENT], "cursor": ""}],
           lambda: list(starkbank.event.query(limit=5, is_delivered=False)))
    record("event.page", [{"events": [EVENT], "cursor": ""}],
           lambda: starkbank.event.page(limit=5)[0])
    record("event.update", [{"event": EVENT}],
           lambda: starkbank.event.update("5764898044149760", is_delivered=True))
    record("event.delete", [{"event": EVENT}],
           lambda: starkbank.event.delete("5764898044149760"))
    record("event.transfer", [{"event": EVENT_TRANSFER}], lambda: starkbank.event.get("4823"))
    record("event.unknown", [{"event": EVENT_UNKNOWN}], lambda: starkbank.event.get("4824"))
    record("event.attempt.get", [{"attempt": EVENT_ATTEMPT}],
           lambda: starkbank.event.attempt.get("1616161616161616"))
    record("event.attempt.query", [{"attempts": [EVENT_ATTEMPT], "cursor": ""}],
           lambda: list(starkbank.event.attempt.query(limit=5,
                                                      event_ids=["5764898044149760"])))
    record("event.attempt.page", [{"attempts": [EVENT_ATTEMPT], "cursor": ""}],
           lambda: starkbank.event.attempt.page(limit=5)[0])

    # post_single, and the reason this case exists: python sends the entity
    # itself as the body, NOT wrapped under "webhook" and NOT a one-element
    # list under "webhooks". The envelope key is on the RESPONSE only. The
    # recorded bodyRaw beside this case is the evidence.
    record("webhook.create", [{"webhook": WEBHOOK}],
           lambda: starkbank.webhook.create(
               url="https://webhook.site/60e9c18e-4b5c-4369-bda1-ab5fcd8e1b29",
               subscriptions=["transfer", "invoice"]))
    record("webhook.get", [{"webhook": WEBHOOK}],
           lambda: starkbank.webhook.get("6178044066660352"))
    record("webhook.query", [{"webhooks": [WEBHOOK], "cursor": ""}],
           lambda: list(starkbank.webhook.query(limit=5)))
    record("webhook.page", [{"webhooks": [WEBHOOK], "cursor": ""}],
           lambda: starkbank.webhook.page(limit=5)[0])
    record("webhook.delete", [{"webhook": WEBHOOK}],
           lambda: starkbank.webhook.delete("6178044066660352"))

    # post_multi with a response whose sub-object's class is chosen by a
    # sibling field. One call, four types, so the mixed batch is what the
    # golden drives rather than four single-type calls that never prove the
    # resolution happens per item.
    previews = [
        starkbank.PaymentPreview(id=BRCODE, scheduled="2026-10-28"),
        starkbank.PaymentPreview(id=BOLETO_LINE, scheduled="2026-10-28"),
        starkbank.PaymentPreview(id=TAX_LINE, scheduled="2026-10-28"),
        starkbank.PaymentPreview(id=UTILITY_LINE, scheduled="2026-10-28"),
    ]
    record("paymentpreview.create",
           [{"previews": [PAYMENT_PREVIEW_BRCODE, PAYMENT_PREVIEW_BOLETO,
                          PAYMENT_PREVIEW_TAX, PAYMENT_PREVIEW_UTILITY]}],
           lambda: starkbank.paymentpreview.create(previews))
    record("paymentpreview.unknown", [{"previews": [PAYMENT_PREVIEW_UNKNOWN]}],
           lambda: starkbank.paymentpreview.create(
               [starkbank.PaymentPreview(id="5656565656565656")]))

    record("balance.get", [{"balances": [BALANCE], "cursor": ""}],
           lambda: starkbank.balance.get())

    utilityPayment = starkbank.UtilityPayment(
        description="Utility Payment - Light Company",
        line="82660000002 8 44361143007 7 41190025511 7 00010601813 8",
        bar_code="82660000002443611430074119002551100010601813",
        scheduled="2026-09-20",
        tags=["electricity", "light"],
    )

    record("utilitypayment.create", [{"payments": [UTILITY_PAYMENT]}],
           lambda: starkbank.utilitypayment.create([utilityPayment]))
    record("utilitypayment.get", [{"payment": UTILITY_PAYMENT}],
           lambda: starkbank.utilitypayment.get("5155165527080960"))
    record("utilitypayment.delete", [{"payment": UTILITY_PAYMENT}],
           lambda: starkbank.utilitypayment.delete("5155165527080960"))
    record("utilitypayment.query", [{"payments": [UTILITY_PAYMENT], "cursor": ""}],
           lambda: list(starkbank.utilitypayment.query(limit=5, status="success",
                                                        tags=["electricity"])))
    record("utilitypayment.page", [{"payments": [UTILITY_PAYMENT], "cursor": ""}],
           lambda: starkbank.utilitypayment.page(limit=5)[0])
    record("utilitypayment.pdf", [PDF],
           lambda: None if starkbank.utilitypayment.pdf("5155165527080960") else None)
    record("utilitypayment.log.get", [{"log": UTILITY_PAYMENT_LOG}],
           lambda: starkbank.utilitypayment.log.get("6341320293482496"))
    record("utilitypayment.log.query", [{"logs": [UTILITY_PAYMENT_LOG], "cursor": ""}],
           lambda: list(starkbank.utilitypayment.log.query(
               limit=5, types=["success"], payment_ids=["5155165527080960"])))
    record("utilitypayment.log.page", [{"logs": [UTILITY_PAYMENT_LOG], "cursor": ""}],
           lambda: starkbank.utilitypayment.log.page(limit=5)[0])

    taxPayment = starkbank.TaxPayment(
        description="ISS Payment - Iron Throne",
        line="85660000006 6 67940064007 5 41190025511 7 00010601813 8",
        bar_code="85660000006679400640074119002551100010601813",
        scheduled="2026-09-21",
        tags=["iss", "throne"],
    )

    record("taxpayment.create", [{"payments": [TAX_PAYMENT]}],
           lambda: starkbank.taxpayment.create([taxPayment]))
    record("taxpayment.get", [{"payment": TAX_PAYMENT}],
           lambda: starkbank.taxpayment.get("5155165527080961"))
    record("taxpayment.delete", [{"payment": TAX_PAYMENT}],
           lambda: starkbank.taxpayment.delete("5155165527080961"))
    record("taxpayment.query", [{"payments": [TAX_PAYMENT], "cursor": ""}],
           lambda: list(starkbank.taxpayment.query(limit=5, status="success",
                                                    tags=["iss"])))
    record("taxpayment.page", [{"payments": [TAX_PAYMENT], "cursor": ""}],
           lambda: starkbank.taxpayment.page(limit=5)[0])
    record("taxpayment.pdf", [PDF],
           lambda: None if starkbank.taxpayment.pdf("5155165527080961") else None)
    record("taxpayment.log.get", [{"log": TAX_PAYMENT_LOG}],
           lambda: starkbank.taxpayment.log.get("6341320293482497"))
    record("taxpayment.log.query", [{"logs": [TAX_PAYMENT_LOG], "cursor": ""}],
           lambda: list(starkbank.taxpayment.log.query(
               limit=5, types=["success"], payment_ids=["5155165527080961"])))
    record("taxpayment.log.page", [{"logs": [TAX_PAYMENT_LOG], "cursor": ""}],
           lambda: starkbank.taxpayment.log.page(limit=5)[0])

    darfPayment = starkbank.DarfPayment(
        description="DARF Payment - competence August",
        revenue_code="5948",
        tax_id="20.018.183/0001-80",
        competence="2026-08-31",
        nominal_amount=23456,
        fine_amount=234,
        interest_amount=456,
        due="2026-10-17",
        reference_number="08.1.17.00-4",
        scheduled="2026-09-22",
        tags=["darf", "federal"],
    )

    record("darfpayment.create", [{"payments": [DARF_PAYMENT]}],
           lambda: starkbank.darfpayment.create([darfPayment]))
    record("darfpayment.get", [{"payment": DARF_PAYMENT}],
           lambda: starkbank.darfpayment.get("5155165527080962"))
    record("darfpayment.delete", [{"payment": DARF_PAYMENT}],
           lambda: starkbank.darfpayment.delete("5155165527080962"))
    record("darfpayment.query", [{"payments": [DARF_PAYMENT], "cursor": ""}],
           lambda: list(starkbank.darfpayment.query(limit=5, status="success",
                                                     tags=["darf"])))
    record("darfpayment.page", [{"payments": [DARF_PAYMENT], "cursor": ""}],
           lambda: starkbank.darfpayment.page(limit=5)[0])
    record("darfpayment.pdf", [PDF],
           lambda: None if starkbank.darfpayment.pdf("5155165527080962") else None)
    record("darfpayment.log.get", [{"log": DARF_PAYMENT_LOG}],
           lambda: starkbank.darfpayment.log.get("6341320293482498"))
    record("darfpayment.log.query", [{"logs": [DARF_PAYMENT_LOG], "cursor": ""}],
           lambda: list(starkbank.darfpayment.log.query(
               limit=5, types=["success"], payment_ids=["5155165527080962"])))
    record("darfpayment.log.page", [{"logs": [DARF_PAYMENT_LOG], "cursor": ""}],
           lambda: starkbank.darfpayment.log.page(limit=5)[0])

    document = {
        "cases": CASES,
        "responses": {
            "invoice": {"invoice": INVOICE},
            "invoices": {"invoices": [INVOICE]},
            "invoicePayment": {"payment": INVOICE_PAYMENT},
            "invoiceLog": {"log": INVOICE_LOG},
            "transfer": {"transfer": TRANSFER},
            "transfers": {"transfers": [TRANSFER]},
            "transferLog": {"log": TRANSFER_LOG},
            "boleto": {"boleto": BOLETO},
            "boletos": {"boletos": [BOLETO]},
            "boletoLog": {"log": BOLETO_LOG},
            "boletoPayment": {"payment": BOLETO_PAYMENT},
            "boletoPayments": {"payments": [BOLETO_PAYMENT]},
            "boletoPaymentLog": {"log": BOLETO_PAYMENT_LOG},
            "brcodePayment": {"payment": BRCODE_PAYMENT},
            "brcodePayments": {"payments": [BRCODE_PAYMENT]},
            "brcodePaymentLog": {"log": BRCODE_PAYMENT_LOG},
            "event": {"event": EVENT},
            "eventTransfer": {"event": EVENT_TRANSFER},
            "eventUnknown": {"event": EVENT_UNKNOWN},
            "eventAttempt": {"attempt": EVENT_ATTEMPT},
            "webhook": {"webhook": WEBHOOK},
            "webhooks": {"webhooks": [WEBHOOK], "cursor": ""},
            "previews": {"previews": [PAYMENT_PREVIEW_BRCODE, PAYMENT_PREVIEW_BOLETO,
                                      PAYMENT_PREVIEW_TAX, PAYMENT_PREVIEW_UTILITY]},
            "previewUnknown": {"previews": [PAYMENT_PREVIEW_UNKNOWN]},
            "balances": {"balances": [BALANCE], "cursor": ""},
            "utilityPayment": {"payment": UTILITY_PAYMENT},
            "utilityPayments": {"payments": [UTILITY_PAYMENT], "cursor": ""},
            "utilityPaymentLog": {"log": UTILITY_PAYMENT_LOG},
            "taxPayment": {"payment": TAX_PAYMENT},
            "taxPayments": {"payments": [TAX_PAYMENT], "cursor": ""},
            "taxPaymentLog": {"log": TAX_PAYMENT_LOG},
            "darfPayment": {"payment": DARF_PAYMENT},
            "darfPayments": {"payments": [DARF_PAYMENT], "cursor": ""},
            "darfPaymentLog": {"log": DARF_PAYMENT_LOG},
        },
    }
    with open(OUT, "w") as handle:
        json.dump(document, handle, indent=2, sort_keys=True)
        handle.write("\n")
    print("recorded {count} cases into {path}".format(count=len(CASES), path=OUT))


if __name__ == "__main__":
    main()
