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
from starkcore.error import Error                             # noqa: E402


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
    if isinstance(value, Error):
        # corporatepurchase.Log.errors: sdk-python's _parse_errors turns each
        # {"code","message"} dict into a starkcore.error.Error (a StarkError
        # subclass, not a SubResource), so cast_json_to_api_format's generic
        # SubResource branch never fires and api_json hands back the raw
        # Error object instead of a plain dict - this is that dict, recovered
        # for comparison purposes only; sdk-python itself never round-trips
        # this field back through json.dumps.
        return {"code": value.code, "message": value.message}
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

DEPOSIT = {
    "id": "5768139429316608",
    "name": "Cosmic Cupcakes",
    "taxId": "20.018.183/0001-80",
    "bankCode": "20018183",
    "branchCode": "1357-9",
    "accountNumber": "876543-2",
    "accountType": "checking",
    "amount": 1234,
    "type": "pix",
    "status": "created",
    "tags": ["reconciliationId", "txId"],
    "fee": 50,
    "transactionIds": ["19827356981273"],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

DEPOSIT_LOG = {
    "id": "6341320293482499",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "credited",
    "errors": [],
    "deposit": DEPOSIT,
}

# tax_id is the masked shape the API returns for a key you do not own -
# checks.py:33's MASKED case, not an encoding failure.
DICT_KEY = {
    "id": "tony@starkbank.com",
    "type": "email",
    "name": "Tony Stark",
    "taxId": "***.345.678-**",
    "ownerType": "naturalPerson",
    "bankName": "Stark Bank",
    "ispb": "20018183",
    "branchCode": "ZW5jcnlwdGVkLWJyYW5jaC1jb2Rl",
    "accountNumber": "ZW5jcnlwdGVkLWFjY291bnQtbnVtYmVy",
    "accountType": "checking",
    "status": "registered",
}

INSTITUTION = {
    "displayName": "Stark Bank",
    "name": "Stark Bank S.A.",
    "spiCode": "20018183",
    "strCode": "123",
}

TRANSACTION = {
    "id": "7656565656565656",
    "amount": 1234,
    "description": "funds redistribution",
    "externalId": "transaction ABC 2026-09-17",
    "receiverId": "5656565656565656",
    "senderId": "5656565656565655",
    "tags": ["abc", "test"],
    "fee": 200,
    "balance": 100000000,
    "source": "transfer/92873912873",
    "created": "2026-09-16T12:00:00+00:00",
}

WORKSPACE = {
    "id": "6284441752174592",
    "username": "starkbankworkspace",
    "name": "Stark Bank Workspace",
    "allowedTaxIds": ["012.345.678-90", "20.018.183/0001-80"],
    "status": "active",
    "organizationId": "5656565656565656",
    "pictureUrl": ("https://storage.googleapis.com/api-ms-workspace-sbx.appspot.com/"
                   "pictures/workspace/6284441752174592.png?20230208220551"),
    "created": "2026-09-16T12:00:00+00:00",
}

PERMISSION = {
    "ownerId": "5656565656565656",
    "ownerType": "project",
    "ownerEmail": "tony@starkbank.com",
    "ownerName": "Tony Stark",
    "ownerPictureUrl": "https://storage.googleapis.com/api-ms-workspace-sbx.appspot.com/pictures/1",
    "ownerStatus": "active",
    "created": "2026-09-16T12:00:00+00:00",
}

CORPORATE_RULE = {
    "id": "5714626427174912",
    "name": "Travel",
    "amount": 200000,
    "interval": "day",
    "schedule": None,
    "currencyCode": "BRL",
    "purposes": ["purchase"],
    "categories": [],
    "countries": [],
    "methods": [],
    "counterAmount": 1000,
    "currencySymbol": "R$",
    "currencyName": "Brazilian Real",
}

CORPORATE_HOLDER = {
    "id": "5729405850615808",
    "name": "Tony Stark",
    "centerId": "5656565656565656",
    "permissions": [PERMISSION],
    "rules": [CORPORATE_RULE],
    "tags": ["iron", "man"],
    "status": "active",
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

CORPORATE_HOLDER_LOG = {
    "id": "6341320293482499",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "created",
    "holder": CORPORATE_HOLDER,
}

CORPORATE_BALANCE = {
    "id": "5992663269507072",
    "amount": 100000,
    "limit": 0,
    "maxLimit": 100000,
    "currency": "BRL",
    "updated": "2026-09-17T12:00:00+00:00",
}

CORPORATE_CARD = {
    "id": "5859665293850624",
    "holderId": "5729405850615808",
    "holderName": "Tony Stark",
    "displayName": "TONY STARK",
    "rules": [CORPORATE_RULE],
    "tags": ["travel"],
    "streetLine1": "Av. Paulista, 200",
    "streetLine2": "Apto. 123",
    "district": "Bela Vista",
    "city": "Sao Paulo",
    "stateCode": "SP",
    "zipCode": "01311-200",
    "type": "virtual",
    "status": "active",
    "number": "123",
    "securityCode": "123",
    "expiration": "2030-01-01T00:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

CORPORATE_CARD_LOG = {
    "id": "6341320293482500",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "created",
    "card": CORPORATE_CARD,
}

CORPORATE_PURCHASE = {
    "id": "5992663269507073",
    "holderId": "5729405850615808",
    "holderName": "Tony Stark",
    "centerId": "5656565656565656",
    "cardId": "5859665293850624",
    "cardEnding": "1234",
    "description": "Swords",
    "amount": 10000,
    "tax": 0,
    "issuerAmount": 10000,
    "issuerCurrencyCode": "BRL",
    "issuerCurrencySymbol": "R$",
    "merchantAmount": 10000,
    "merchantCurrencyCode": "BRL",
    "merchantCurrencySymbol": "R$",
    "merchantCategoryCode": "fastFoodRestaurants",
    "merchantCategoryType": "food",
    "merchantCountryCode": "BRA",
    "merchantName": "Iron Bank S.A.",
    "merchantDisplayName": "Iron Bank",
    "merchantDisplayUrl": "https://starkbank.com",
    "merchantFee": 0,
    "methodCode": "chip",
    "tags": ["war"],
    "corporateTransactionIds": ["corporate-purchase/5992663269507073"],
    "status": "approved",
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

CORPORATE_PURCHASE_LOG = {
    "id": "6341320293482501",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "approved",
    "errors": [{"code": "invalidPin", "message": "PIN is invalid"}],
    "description": "Approved",
    "corporateTransactionId": "corporate-purchase/5992663269507073",
    "purchase": CORPORATE_PURCHASE,
}

CORPORATE_INVOICE = {
    "id": "5155165527080963",
    "amount": 100000,
    "taxId": "20.018.183/0001-80",
    "name": "Iron Bank S.A.",
    "tags": ["load"],
    "brcode": "00020101021226800014br.gov.bcb.pix2571brcode-h",
    "due": "2026-10-28T17:59:26+00:00",
    "link": "https://my-workspace.sandbox.starkbank.com/invoicelink/abc",
    "status": "created",
    "corporateTransactionId": "",
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

CORPORATE_TRANSACTION = {
    "id": "5155165527080964",
    "amount": 100000,
    "balance": 200000,
    "description": "Buying food",
    "source": "corporate-purchase/5992663269507073",
    "tags": ["tony", "stark"],
    "created": "2026-09-16T12:00:00+00:00",
}

CORPORATE_WITHDRAWAL = {
    "id": "5155165527080965",
    "amount": 100000,
    "externalId": "12345",
    "tags": ["cash"],
    "transactionId": "transaction/5155165527080965",
    "corporateTransactionId": "corporate-withdrawal/5155165527080965",
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

MERCHANT_SESSION = {
    "id": "6234161654976512",
    "uuid": "901e71f2447c43c886f58366a5432c4b",
    "allowedFundingTypes": ["credit", "debit"],
    "allowedInstallments": [{"totalAmount": 100, "count": 1},
                            {"totalAmount": 51, "count": 2}],
    "allowedIps": [],
    "challengeMode": "enabled",
    "expiration": 3600,
    "holderId": "",
    "softDescriptor": "",
    "status": "created",
    "tags": ["labs"],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-16T12:00:00+00:00",
}

MERCHANT_SESSION_LOG = {
    "id": "6357482625564672",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "created",
    "errors": ["expiredSession"],
    "session": MERCHANT_SESSION,
}

SESSION_PURCHASE = {
    "id": "6234161654976513",
    "amount": 1000,
    "cardExpiration": "2032-12",
    "cardNumber": "",
    "cardSecurityCode": "",
    "holderName": "Tony Stark",
    "fundingType": "credit",
    "holderEmail": "tony@starkbank.com",
    "holderPhone": "11999998888",
    "holderId": "",
    "installmentCount": 1,
    "billingCountryCode": "BRA",
    "billingCity": "Sao Paulo",
    "billingStateCode": "SP",
    "billingStreetLine1": "Av. Paulista, 200",
    "billingStreetLine2": "",
    "billingZipCode": "01311-200",
    "metadata": {"userAgent": "python-requests", "timezoneOffset": 180,
                "userIp": "191.9.0.0", "language": "pt-BR"},
    "cardEnding": "1234",
    "cardId": "5629632759480320",
    "challengeMode": "enabled",
    "challengeUrl": "https://sandbox.starkbank.com/challenge/abc",
    "currencyCode": "BRL",
    "endToEndId": "E79457883202101262140HHX553UPqeq",
    "fee": 0,
    "network": "mastercard",
    "source": "merchant-session/901e71f2447c43c886f58366a5432c4b",
    "softDescriptor": "starkbank",
    "status": "approved",
    "tags": [],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-16T12:00:00+00:00",
}

MERCHANT_CARD = {
    "id": "5629632759480320",
    "ending": "1234",
    "fundingType": "credit",
    "holderName": "Tony Stark",
    "network": "mastercard",
    "status": "active",
    "tags": ["labs"],
    "expiration": "2032-12-01T00:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

MERCHANT_CARD_LOG = {
    "id": "5629632759480321",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
    "type": "created",
    "errors": [{"code": "invalidCvv", "message": "CVV is invalid"}],
    "card": MERCHANT_CARD,
}

MERCHANT_INSTALLMENT = {
    "id": "5629632759480322",
    "amount": 5000,
    "due": "2026-10-28T00:00:00+00:00",
    "fee": 100,
    "fundingType": "credit",
    "network": "mastercard",
    "purchaseId": "5629632759480323",
    "status": "created",
    "tags": ["labs"],
    "transactionIds": [],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

MERCHANT_INSTALLMENT_LOG = {
    "id": "5629632759480324",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
    "type": "created",
    "errors": [{"code": "insufficientFunds", "message": "Insufficient funds"}],
    "installment": MERCHANT_INSTALLMENT,
}

MERCHANT_PURCHASE = {
    "id": "5629632759480323",
    "amount": 10000,
    "cardId": "5629632759480320",
    "fundingType": "credit",
    "installmentCount": 1,
    "cardExpiration": "",
    "cardNumber": "",
    "cardSecurityCode": "",
    "holderName": "",
    "holderEmail": "",
    "holderPhone": "",
    "holderId": "",
    "billingCountryCode": "",
    "billingCity": "",
    "billingStateCode": "",
    "billingStreetLine1": "",
    "billingStreetLine2": "",
    "billingZipCode": "",
    "metadata": {},
    "cardEnding": "1234",
    "softDescriptor": "starkbank",
    "challengeMode": "enabled",
    "challengeUrl": "",
    "currencyCode": "BRL",
    "endToEndId": "",
    "fee": 200,
    "network": "mastercard",
    "source": "merchant-session/901e71f2447c43c886f58366a5432c4b",
    "status": "approved",
    "tags": ["labs"],
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

MERCHANT_PURCHASE_LOG = {
    "id": "5629632759480325",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "approved",
    "errors": [{"code": "invalidCvv", "message": "CVV is invalid"}],
    "purchase": MERCHANT_PURCHASE,
}

CARD_METHOD = {"code": "chip", "name": "Chip", "number": "05"}
MERCHANT_CATEGORY = {"code": "fastFoodRestaurants", "type": "food",
                     "name": "Fast food restaurants", "number": "5814"}
MERCHANT_COUNTRY = {"code": "BRA", "name": "Brazil", "number": "076", "shortCode": "BR"}

BOLETO_HOLMES = {
    "id": "6234161654976514",
    "boletoId": "5155165527080960",
    "tags": ["sherlock"],
    "status": "solved",
    "result": "paid",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

BOLETO_HOLMES_LOG = {
    "id": "6234161654976515",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
    "type": "solved",
    "holmes": BOLETO_HOLMES,
}

DYNAMIC_BRCODE = {
    "id": "901e71f2447c43c886f58366a5432c4b",
    "uuid": "901e71f2447c43c886f58366a5432c4b",
    "amount": 100000,
    "expiration": 3600,
    "displayDescription": "Payment for service #1234",
    "rules": [{"key": "allowedTaxIds", "value": ["012.345.678-90"]}],
    "tags": ["dynamic"],
    "pictureUrl": "https://sandbox.starkbank.com/qr/901e71f2447c43c886f58366a5432c4b.png",
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

INVOICE_PULL_SUBSCRIPTION = {
    "id": "6234161654976516",
    "start": "2026-09-16",
    "interval": "month",
    "pullMode": "automatic",
    "pullRetryLimit": 3,
    "type": "qrcode",
    "amount": 100000,
    "amountMinLimit": 0,
    "displayDescription": "Subscription payment",
    "due": "2026-09-18",
    "externalId": "my-external-id",
    "referenceCode": "REF123456",
    "end": "2027-09-16",
    "data": {},
    "name": "Iron Bank S.A.",
    "taxId": "20.018.183/0001-80",
    "tags": ["subscription"],
    "status": "active",
    "bacenId": "RR2001818320250616dtsPkBVaBYs",
    "brcode": "00020101021126580014br.gov.bcb.pix",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

INVOICE_PULL_SUBSCRIPTION_LOG = {
    "id": "6234161654976517",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "active",
    "errors": [{"code": "invalidStart", "message": "start date is invalid"}],
    "subscription": INVOICE_PULL_SUBSCRIPTION,
}

INVOICE_PULL_REQUEST = {
    "id": "6234161654976518",
    "subscriptionId": "6234161654976516",
    "invoiceId": "5155165527080960",
    "due": "2026-10-28T17:59:26+00:00",
    "attemptType": "default",
    "tags": ["pull"],
    "externalId": "my-external-id-2",
    "displayDescription": "Payment for services",
    "status": "success",
    "installmentId": "6234161654976519",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

INVOICE_PULL_REQUEST_LOG = {
    "id": "6234161654976520",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [{"code": "insufficientFunds", "message": "insufficient funds"}],
    "request": INVOICE_PULL_REQUEST,
}

PAYMENT_REQUEST_TRANSFER = {
    "id": "6234161654976521",
    "centerId": "5656565656565656",
    "payment": TRANSFER,
    "type": "transfer",
    "due": "2026-09-20",
    "tags": ["urgent"],
    "amount": 1000,
    "description": "Tony Stark's Suit",
    "status": "pending",
    "actions": [{"type": "member", "id": "5656565656565656", "action": "requested"}],
    "updated": "2026-09-17T12:00:00+00:00",
    "created": "2026-09-16T12:00:00+00:00",
}

# The second payment type this SDK's polymorphic hydration is exercised
# against, alongside PAYMENT_REQUEST_TRANSFER - see paymentrequest.h.
PAYMENT_REQUEST_BOLETO_PAYMENT = dict(
    PAYMENT_REQUEST_TRANSFER,
    id="6234161654976522",
    payment=BOLETO_PAYMENT,
    type="boleto-payment",
    amount=100000,
)

VERIFIED_ACCOUNT = {
    "id": "6155165527080960",
    "taxId": "20.018.183/0001-80",
    "bankCode": "20018183",
    "branchCode": "1357-9",
    "keyId": "tony@starkbank.com",
    "name": "Anthony Edward Stark",
    "number": "876543-2",
    "type": "checking",
    "tags": ["employees", "monthly"],
    "bankName": "Stark Bank",
    "status": "active",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

VERIFIED_ACCOUNT_LOG = {
    "id": "6341320293482502",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "active",
    "errors": [{"code": "invalidKey", "message": "Pix key is invalid"}],
    "account": VERIFIED_ACCOUNT,
}

VERIFIED_TRANSFER = {
    "id": "6155165527080961",
    "amount": 1234,
    "accountId": "6155165527080960",
    "externalId": "my-internal-id-654321",
    "scheduled": "2026-10-28T17:59:26+00:00",
    "description": "Payment for service #1234",
    "displayDescription": "Sword sharpening",
    "tags": ["arya", "stark"],
    "rules": [{"key": "resendingLimit", "value": 5}],
    "fee": 200,
    "status": "processing",
    "transactionIds": ["19827356981274"],
    "metadata": {"tracker": "xyz", "attempt": 1},
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

SPLIT_RECEIVER = {
    "id": "6155165527080962",
    "name": "Anthony Edward Stark",
    "taxId": "20.018.183/0001-80",
    "bankCode": "20018183",
    "branchCode": "1357-9",
    "accountNumber": "876543-2",
    "accountType": "checking",
    "tags": ["seller/123456"],
    "status": "success",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

SPLIT_RECEIVER_LOG = {
    "id": "6341320293482503",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "receiver": SPLIT_RECEIVER,
}

SPLIT_PROFILE = {
    "id": "6155165527080963",
    "delay": 604800,
    "interval": "week",
    "tags": ["default"],
    "status": "created",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

SPLIT_PROFILE_LOG = {
    "id": "6341320293482504",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "created",
    "errors": [],
    "profile": SPLIT_PROFILE,
}

SPLIT = {
    "id": "6155165527080964",
    "amount": 141,
    "receiverId": "5706627130851328",
    "externalId": "invoice/1234/receiver/5678",
    "tags": ["war", "supply"],
    "scheduled": "2026-09-16T15:17:03+00:00",
    "source": "5155165527080960",
    "status": "success",
    "created": "2026-09-16T12:00:00+00:00",
    "updated": "2026-09-17T12:00:00+00:00",
}

SPLIT_LOG = {
    "id": "6341320293482505",
    "created": "2026-09-16T12:00:00+00:00",
    "type": "success",
    "errors": [],
    "split": SPLIT,
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

    record("deposit.get", [{"deposit": DEPOSIT}],
           lambda: starkbank.deposit.get("5768139429316608"))
    record("deposit.query", [{"deposits": [DEPOSIT], "cursor": ""}],
           lambda: list(starkbank.deposit.query(limit=5, status="created",
                                                tags=["reconciliationId", "txId"])))
    record("deposit.page", [{"deposits": [DEPOSIT], "cursor": ""}],
           lambda: starkbank.deposit.page(limit=5)[0])
    # amount=0: a full reversal, and the case that proves the engine tells a
    # legal zero apart from an absent key at every layer, not only Invoice's.
    record("deposit.update", [{"deposit": DEPOSIT}],
           lambda: starkbank.deposit.update("5768139429316608", amount=0))
    record("deposit.log.get", [{"log": DEPOSIT_LOG}],
           lambda: starkbank.deposit.log.get("6341320293482499"))
    record("deposit.log.query", [{"logs": [DEPOSIT_LOG], "cursor": ""}],
           lambda: list(starkbank.deposit.log.query(limit=5, types=["credited"],
                                                     deposit_ids=["5768139429316608"])))
    record("deposit.log.page", [{"logs": [DEPOSIT_LOG], "cursor": ""}],
           lambda: starkbank.deposit.log.page(limit=5)[0])
    record("deposit.log.pdf", [PDF],
           lambda: None if starkbank.deposit.log.pdf("6341320293482499") else None)

    record("dictkey.get", [{"key": DICT_KEY}],
           lambda: starkbank.dictkey.get("tony@starkbank.com"))
    record("dictkey.query", [{"keys": [DICT_KEY], "cursor": ""}],
           lambda: list(starkbank.dictkey.query(limit=5, type="email", status="registered")))
    record("dictkey.page", [{"keys": [DICT_KEY], "cursor": ""}],
           lambda: starkbank.dictkey.page(limit=5)[0])

    # institution.query is rest.get_page(...)[0]: one page, cursor discarded,
    # no looping - the golden proves the request python actually sends, and
    # only the request: python's own return value carries no cursor to check.
    record("institution.query", [{"institutions": [INSTITUTION], "cursor": "c1"}],
           lambda: starkbank.institution.query(limit=5, search="stark",
                                               spi_codes=["20018183"]))

    record("transaction.get", [{"transaction": TRANSACTION}],
           lambda: starkbank.transaction.get("7656565656565656"))
    record("transaction.query", [{"transactions": [TRANSACTION], "cursor": ""}],
           lambda: list(starkbank.transaction.query(limit=5, tags=["abc", "test"],
                                                     external_ids=["transaction ABC 2026-09-17"])))
    record("transaction.page", [{"transactions": [TRANSACTION], "cursor": ""}],
           lambda: starkbank.transaction.page(limit=5)[0])

    # post_single, like webhook.create: the Workspace itself is the body, not
    # wrapped in a list.
    record("workspace.create", [{"workspace": WORKSPACE}],
           lambda: starkbank.workspace.create(
               username="starkbankworkspace", name="Stark Bank Workspace",
               allowed_tax_ids=["012.345.678-90", "20.018.183/0001-80"]))
    record("workspace.get", [{"workspace": WORKSPACE}],
           lambda: starkbank.workspace.get("6284441752174592"))
    record("workspace.query", [{"workspaces": [WORKSPACE], "cursor": ""}],
           lambda: list(starkbank.workspace.query(limit=5, username="starkbankworkspace")))
    record("workspace.page", [{"workspaces": [WORKSPACE], "cursor": ""}],
           lambda: starkbank.workspace.page(limit=5)[0])
    # picture is base64-encoded into one data-uri string by python itself;
    # the golden's bodyRaw is the evidence sdk-c's caller must reproduce byte
    # for byte when it builds that same string. username and name are also
    # patched here so the golden's query/queryRaw prove core-python's
    # patch_id(**query) echo: they land both in the body and in the URL as
    # ?name=...&username=... - status and picture do not.
    record("workspace.update", [{"workspace": WORKSPACE}],
           lambda: starkbank.workspace.update(
               "6284441752174592", username="starkbankworkspace",
               name="Stark Bank Workspace", status="active",
               picture=b"\x89PNG\r\n\x1a\n fake", picture_type="image/png"))

    holder = starkbank.CorporateHolder(
        name="Tony Stark",
        center_id="5656565656565656",
        tags=["iron", "man"],
    )
    record("corporateholder.create", [{"holders": [CORPORATE_HOLDER]}],
           lambda: starkbank.corporateholder.create([holder]))
    record("corporateholder.get", [{"holder": CORPORATE_HOLDER}],
           lambda: starkbank.corporateholder.get("5729405850615808"))
    record("corporateholder.query", [{"holders": [CORPORATE_HOLDER], "cursor": ""}],
           lambda: list(starkbank.corporateholder.query(limit=5, status="active",
                                                         tags=["iron"])))
    record("corporateholder.page", [{"holders": [CORPORATE_HOLDER], "cursor": ""}],
           lambda: starkbank.corporateholder.page(limit=5)[0])
    record("corporateholder.update", [{"holder": CORPORATE_HOLDER}],
           lambda: starkbank.corporateholder.update("5729405850615808", status="blocked",
                                                     tags=["iron"]))
    record("corporateholder.delete", [{"holder": CORPORATE_HOLDER}],
           lambda: starkbank.corporateholder.cancel("5729405850615808"))
    record("corporateholder.log.get", [{"log": CORPORATE_HOLDER_LOG}],
           lambda: starkbank.corporateholder.log.get("6341320293482499"))
    record("corporateholder.log.query", [{"logs": [CORPORATE_HOLDER_LOG], "cursor": ""}],
           lambda: list(starkbank.corporateholder.log.query(
               limit=5, types=["created"], holder_ids=["5729405850615808"])))
    record("corporateholder.log.page", [{"logs": [CORPORATE_HOLDER_LOG], "cursor": ""}],
           lambda: starkbank.corporateholder.log.page(limit=5)[0])

    record("corporatebalance.get", [{"balances": [CORPORATE_BALANCE], "cursor": ""}],
           lambda: starkbank.corporatebalance.get())

    card = starkbank.CorporateCard(holder_id="5729405850615808")
    record("corporatecard.create", [{"card": CORPORATE_CARD}],
           lambda: starkbank.corporatecard.create(card))
    record("corporatecard.get", [{"card": CORPORATE_CARD}],
           lambda: starkbank.corporatecard.get("5859665293850624"))
    record("corporatecard.query", [{"cards": [CORPORATE_CARD], "cursor": ""}],
           lambda: list(starkbank.corporatecard.query(limit=5, status="active",
                                                       tags=["travel"])))
    record("corporatecard.page", [{"cards": [CORPORATE_CARD], "cursor": ""}],
           lambda: starkbank.corporatecard.page(limit=5)[0])
    record("corporatecard.update", [{"card": CORPORATE_CARD}],
           lambda: starkbank.corporatecard.update("5859665293850624", status="blocked",
                                                   tags=["travel"]))
    record("corporatecard.delete", [{"card": CORPORATE_CARD}],
           lambda: starkbank.corporatecard.cancel("5859665293850624"))
    record("corporatecard.log.get", [{"log": CORPORATE_CARD_LOG}],
           lambda: starkbank.corporatecard.log.get("6341320293482500"))
    record("corporatecard.log.query", [{"logs": [CORPORATE_CARD_LOG], "cursor": ""}],
           lambda: list(starkbank.corporatecard.log.query(
               limit=5, types=["created"], card_ids=["5859665293850624"])))
    record("corporatecard.log.page", [{"logs": [CORPORATE_CARD_LOG], "cursor": ""}],
           lambda: starkbank.corporatecard.log.page(limit=5)[0])

    record("corporatepurchase.get", [{"purchase": CORPORATE_PURCHASE}],
           lambda: starkbank.corporatepurchase.get("5992663269507073"))
    record("corporatepurchase.query", [{"purchases": [CORPORATE_PURCHASE], "cursor": ""}],
           lambda: list(starkbank.corporatepurchase.query(
               limit=5, status="approved", holder_ids=["5729405850615808"])))
    # starkbank.corporatepurchase.__init__ exports only query/get/parse/response;
    # page() is defined in __corporatepurchase.py (drift.py's AST reader sees
    # it, matching this table's PAGE verb) but is not re-exported at package
    # level - reached here through the private module directly rather than
    # through starkbank.corporatepurchase.page, which does not exist.
    from starkbank.corporatepurchase import __corporatepurchase as _corporatepurchase
    record("corporatepurchase.page", [{"purchases": [CORPORATE_PURCHASE], "cursor": ""}],
           lambda: _corporatepurchase.page(limit=5)[0])
    record("corporatepurchase.log.get", [{"log": CORPORATE_PURCHASE_LOG}],
           lambda: starkbank.corporatepurchase.log.get("6341320293482501"))
    record("corporatepurchase.log.query", [{"logs": [CORPORATE_PURCHASE_LOG], "cursor": ""}],
           lambda: list(starkbank.corporatepurchase.log.query(
               limit=5, types=["approved"], purchase_ids=["5992663269507073"])))
    record("corporatepurchase.log.page", [{"logs": [CORPORATE_PURCHASE_LOG], "cursor": ""}],
           lambda: starkbank.corporatepurchase.log.page(limit=5)[0])

    invoice2 = starkbank.CorporateInvoice(amount=100000, tags=["load"])
    record("corporateinvoice.create", [{"invoice": CORPORATE_INVOICE}],
           lambda: starkbank.corporateinvoice.create(invoice2))
    record("corporateinvoice.query", [{"invoices": [CORPORATE_INVOICE], "cursor": ""}],
           lambda: list(starkbank.corporateinvoice.query(limit=5, status="created",
                                                          tags=["load"])))
    record("corporateinvoice.page", [{"invoices": [CORPORATE_INVOICE], "cursor": ""}],
           lambda: starkbank.corporateinvoice.page(limit=5)[0])

    record("corporatetransaction.get", [{"transaction": CORPORATE_TRANSACTION}],
           lambda: starkbank.corporatetransaction.get("5155165527080964"))
    record("corporatetransaction.query", [{"transactions": [CORPORATE_TRANSACTION],
                                           "cursor": ""}],
           lambda: list(starkbank.corporatetransaction.query(
               limit=5, tags=["tony"], external_ids=["19827356981276"],
               ids=["5155165527080964"], source="corporate-purchase/5992663269507073")))
    record("corporatetransaction.page", [{"transactions": [CORPORATE_TRANSACTION],
                                          "cursor": ""}],
           lambda: starkbank.corporatetransaction.page(limit=5)[0])

    withdrawal = starkbank.CorporateWithdrawal(amount=100000, external_id="12345",
                                               tags=["cash"])
    record("corporatewithdrawal.create", [{"withdrawal": CORPORATE_WITHDRAWAL}],
           lambda: starkbank.corporatewithdrawal.create(withdrawal))
    record("corporatewithdrawal.get", [{"withdrawal": CORPORATE_WITHDRAWAL}],
           lambda: starkbank.corporatewithdrawal.get("5155165527080965"))
    record("corporatewithdrawal.query", [{"withdrawals": [CORPORATE_WITHDRAWAL],
                                          "cursor": ""}],
           lambda: list(starkbank.corporatewithdrawal.query(
               limit=5, tags=["cash"], external_ids=["12345"])))
    record("corporatewithdrawal.page", [{"withdrawals": [CORPORATE_WITHDRAWAL],
                                         "cursor": ""}],
           lambda: starkbank.corporatewithdrawal.page(limit=5)[0])

    session = starkbank.MerchantSession(
        allowed_funding_types=["credit", "debit"],
        allowed_installments=[
            starkbank.merchantsession.AllowedInstallment(total_amount=100, count=1),
            starkbank.merchantsession.AllowedInstallment(total_amount=51, count=2),
        ],
        expiration=3600,
        tags=["labs"],
    )
    record("merchantsession.create", [{"session": MERCHANT_SESSION}],
           lambda: starkbank.merchantsession.create(session))
    record("merchantsession.get", [{"session": MERCHANT_SESSION}],
           lambda: starkbank.merchantsession.get("6234161654976512"))
    record("merchantsession.query", [{"sessions": [MERCHANT_SESSION], "cursor": ""}],
           lambda: list(starkbank.merchantsession.query(
               limit=5, status="created", tags=["labs"],
               holder_id="5729405850615808")))
    record("merchantsession.page", [{"sessions": [MERCHANT_SESSION], "cursor": ""}],
           lambda: starkbank.merchantsession.page(limit=5)[0])
    session_purchase = starkbank.merchantsession.Purchase(
        amount=1000,
        card_expiration="2032-12",
        card_number="5579433276352001",
        card_security_code="123",
        holder_name="Tony Stark",
        funding_type="credit",
        holder_email="tony@starkbank.com",
        holder_phone="11999998888",
        installment_count=1,
        billing_country_code="BRA",
        billing_city="Sao Paulo",
        billing_state_code="SP",
        billing_street_line_1="Av. Paulista, 200",
        billing_zip_code="01311-200",
        metadata={"userAgent": "python-requests", "timezoneOffset": 180,
                  "userIp": "191.9.0.0", "language": "pt-BR"},
    )
    record("merchantsession.purchase", [{"purchase": SESSION_PURCHASE}],
           lambda: starkbank.merchantsession.purchase(
               "901e71f2447c43c886f58366a5432c4b", session_purchase))
    record("merchantsession.log.get", [{"log": MERCHANT_SESSION_LOG}],
           lambda: starkbank.merchantsession.log.get("6357482625564672"))
    record("merchantsession.log.query", [{"logs": [MERCHANT_SESSION_LOG], "cursor": ""}],
           lambda: list(starkbank.merchantsession.log.query(
               limit=5, types=["created"], session_ids=["6234161654976512"])))
    record("merchantsession.log.page", [{"logs": [MERCHANT_SESSION_LOG], "cursor": ""}],
           lambda: starkbank.merchantsession.log.page(limit=5)[0])

    record("merchantcard.get", [{"card": MERCHANT_CARD}],
           lambda: starkbank.merchantcard.get("5629632759480320"))
    record("merchantcard.query", [{"cards": [MERCHANT_CARD], "cursor": ""}],
           lambda: list(starkbank.merchantcard.query(limit=5, status="active",
                                                      tags=["labs"])))
    record("merchantcard.page", [{"cards": [MERCHANT_CARD], "cursor": ""}],
           lambda: starkbank.merchantcard.page(limit=5)[0])
    record("merchantcard.log.get", [{"log": MERCHANT_CARD_LOG}],
           lambda: starkbank.merchantcard.log.get("5629632759480321"))
    record("merchantcard.log.query", [{"logs": [MERCHANT_CARD_LOG], "cursor": ""}],
           lambda: list(starkbank.merchantcard.log.query(
               limit=5, card_ids=["5629632759480320"], types=["created"])))
    record("merchantcard.log.page", [{"logs": [MERCHANT_CARD_LOG], "cursor": ""}],
           lambda: starkbank.merchantcard.log.page(limit=5)[0])

    record("merchantinstallment.get", [{"installment": MERCHANT_INSTALLMENT}],
           lambda: starkbank.merchantinstallment.get("5629632759480322"))
    record("merchantinstallment.query", [{"installments": [MERCHANT_INSTALLMENT],
                                          "cursor": ""}],
           lambda: list(starkbank.merchantinstallment.query(
               limit=5, status="created", tags=["labs"],
               purchase_ids=["5629632759480323"])))
    record("merchantinstallment.page", [{"installments": [MERCHANT_INSTALLMENT],
                                         "cursor": ""}],
           lambda: starkbank.merchantinstallment.page(limit=5)[0])
    record("merchantinstallment.log.get", [{"log": MERCHANT_INSTALLMENT_LOG}],
           lambda: starkbank.merchantinstallment.log.get("5629632759480324"))
    record("merchantinstallment.log.query", [{"logs": [MERCHANT_INSTALLMENT_LOG],
                                              "cursor": ""}],
           lambda: list(starkbank.merchantinstallment.log.query(
               limit=5, types=["created"], installment_ids=["5629632759480322"])))
    record("merchantinstallment.log.page", [{"logs": [MERCHANT_INSTALLMENT_LOG],
                                             "cursor": ""}],
           lambda: starkbank.merchantinstallment.log.page(limit=5)[0])

    purchase2 = starkbank.MerchantPurchase(
        amount=10000,
        card_id="5629632759480320",
        funding_type="credit",
        installment_count=1,
        tags=["labs"],
    )
    record("merchantpurchase.create", [{"purchase": MERCHANT_PURCHASE}],
           lambda: starkbank.merchantpurchase.create(purchase2))
    record("merchantpurchase.get", [{"purchase": MERCHANT_PURCHASE}],
           lambda: starkbank.merchantpurchase.get("5629632759480323"))
    record("merchantpurchase.query", [{"purchases": [MERCHANT_PURCHASE], "cursor": ""}],
           lambda: list(starkbank.merchantpurchase.query(
               limit=5, status="approved", tags=["labs"],
               holder_id="5729405850615808")))
    record("merchantpurchase.page", [{"purchases": [MERCHANT_PURCHASE], "cursor": ""}],
           lambda: starkbank.merchantpurchase.page(limit=5)[0])
    record("merchantpurchase.update", [{"purchase": MERCHANT_PURCHASE}],
           lambda: starkbank.merchantpurchase.update("5629632759480323",
                                                      status="canceled", amount=0))
    record("merchantpurchase.log.get", [{"log": MERCHANT_PURCHASE_LOG}],
           lambda: starkbank.merchantpurchase.log.get("5629632759480325"))
    record("merchantpurchase.log.query", [{"logs": [MERCHANT_PURCHASE_LOG], "cursor": ""}],
           lambda: list(starkbank.merchantpurchase.log.query(
               limit=5, types=["approved"], purchase_ids=["5629632759480323"])))
    record("merchantpurchase.log.page", [{"logs": [MERCHANT_PURCHASE_LOG], "cursor": ""}],
           lambda: starkbank.merchantpurchase.log.page(limit=5)[0])

    record("cardmethod.query", [{"methods": [CARD_METHOD], "cursor": ""}],
           lambda: list(starkbank.cardmethod.query(search="chip")))
    record("merchantcategory.query", [{"categories": [MERCHANT_CATEGORY], "cursor": ""}],
           lambda: list(starkbank.merchantcategory.query(search="food")))
    record("merchantcountry.query", [{"countries": [MERCHANT_COUNTRY], "cursor": ""}],
           lambda: list(starkbank.merchantcountry.query(search="brazil")))

    holmes = starkbank.BoletoHolmes(boleto_id="5155165527080960", tags=["sherlock"])
    record("boletoholmes.create", [{"holmes": [BOLETO_HOLMES]}],
           lambda: starkbank.boletoholmes.create([holmes]))
    record("boletoholmes.get", [{"holmes": BOLETO_HOLMES}],
           lambda: starkbank.boletoholmes.get("6234161654976514"))
    record("boletoholmes.query", [{"holmes": [BOLETO_HOLMES], "cursor": ""}],
           lambda: list(starkbank.boletoholmes.query(limit=5, status="solved",
                                                      boleto_id="5155165527080960")))
    record("boletoholmes.page", [{"holmes": [BOLETO_HOLMES], "cursor": ""}],
           lambda: starkbank.boletoholmes.page(limit=5)[0])
    record("boletoholmes.log.get", [{"log": BOLETO_HOLMES_LOG}],
           lambda: starkbank.boletoholmes.log.get("6234161654976515"))
    record("boletoholmes.log.query", [{"logs": [BOLETO_HOLMES_LOG], "cursor": ""}],
           lambda: list(starkbank.boletoholmes.log.query(
               limit=5, types=["solved"], holmes_ids=["6234161654976514"])))
    record("boletoholmes.log.page", [{"logs": [BOLETO_HOLMES_LOG], "cursor": ""}],
           lambda: starkbank.boletoholmes.log.page(limit=5)[0])

    brcode = starkbank.DynamicBrcode(
        amount=100000,
        expiration=3600,
        display_description="Payment for service #1234",
        rules=[starkbank.dynamicbrcode.Rule(key="allowedTaxIds",
                                            value=["012.345.678-90"])],
        tags=["dynamic"],
    )
    record("dynamicbrcode.create", [{"brcodes": [DYNAMIC_BRCODE]}],
           lambda: starkbank.dynamicbrcode.create([brcode]))
    record("dynamicbrcode.get", [{"brcode": DYNAMIC_BRCODE}],
           lambda: starkbank.dynamicbrcode.get("901e71f2447c43c886f58366a5432c4b"))
    record("dynamicbrcode.query", [{"brcodes": [DYNAMIC_BRCODE], "cursor": ""}],
           lambda: list(starkbank.dynamicbrcode.query(limit=5, tags=["dynamic"])))
    record("dynamicbrcode.page", [{"brcodes": [DYNAMIC_BRCODE], "cursor": ""}],
           lambda: starkbank.dynamicbrcode.page(limit=5)[0])

    subscription = starkbank.InvoicePullSubscription(
        start="2026-09-16",
        interval="month",
        pull_mode="automatic",
        pull_retry_limit=3,
        type="qrcode",
        amount=100000,
        tags=["subscription"],
    )
    record("invoicepullsubscription.create", [{"subscriptions": [INVOICE_PULL_SUBSCRIPTION]}],
           lambda: starkbank.invoicepullsubscription.create([subscription]))
    record("invoicepullsubscription.get", [{"subscription": INVOICE_PULL_SUBSCRIPTION}],
           lambda: starkbank.invoicepullsubscription.get("6234161654976516"))
    record("invoicepullsubscription.query", [{"subscriptions": [INVOICE_PULL_SUBSCRIPTION],
                                              "cursor": ""}],
           lambda: list(starkbank.invoicepullsubscription.query(
               limit=5, status=["active"], tags=["subscription"])))
    record("invoicepullsubscription.page", [{"subscriptions": [INVOICE_PULL_SUBSCRIPTION],
                                             "cursor": ""}],
           lambda: starkbank.invoicepullsubscription.page(limit=5)[0])
    record("invoicepullsubscription.delete", [{"subscription": INVOICE_PULL_SUBSCRIPTION}],
           lambda: starkbank.invoicepullsubscription.cancel("6234161654976516"))
    record("invoicepullsubscription.log.get", [{"log": INVOICE_PULL_SUBSCRIPTION_LOG}],
           lambda: starkbank.invoicepullsubscription.log.get("6234161654976517"))
    record("invoicepullsubscription.log.query", [{"logs": [INVOICE_PULL_SUBSCRIPTION_LOG],
                                                  "cursor": ""}],
           lambda: list(starkbank.invoicepullsubscription.log.query(
               limit=5, types=["active"], subscription_ids=["6234161654976516"])))
    record("invoicepullsubscription.log.page", [{"logs": [INVOICE_PULL_SUBSCRIPTION_LOG],
                                                 "cursor": ""}],
           lambda: starkbank.invoicepullsubscription.log.page(limit=5)[0])

    pull_request = starkbank.InvoicePullRequest(
        subscription_id="6234161654976516",
        invoice_id="5155165527080960",
        due="2026-10-28T17:59:26+00:00",
        tags=["pull"],
    )
    record("invoicepullrequest.create", [{"requests": [INVOICE_PULL_REQUEST]}],
           lambda: starkbank.invoicepullrequest.create([pull_request]))
    record("invoicepullrequest.get", [{"request": INVOICE_PULL_REQUEST}],
           lambda: starkbank.invoicepullrequest.get("6234161654976518"))
    record("invoicepullrequest.query", [{"requests": [INVOICE_PULL_REQUEST], "cursor": ""}],
           lambda: list(starkbank.invoicepullrequest.query(
               limit=5, status=["success"], subscription_ids=["6234161654976516"])))
    record("invoicepullrequest.page", [{"requests": [INVOICE_PULL_REQUEST], "cursor": ""}],
           lambda: starkbank.invoicepullrequest.page(limit=5)[0])
    record("invoicepullrequest.delete", [{"request": INVOICE_PULL_REQUEST}],
           lambda: starkbank.invoicepullrequest.cancel("6234161654976518"))
    record("invoicepullrequest.log.get", [{"log": INVOICE_PULL_REQUEST_LOG}],
           lambda: starkbank.invoicepullrequest.log.get("6234161654976520"))
    record("invoicepullrequest.log.query", [{"logs": [INVOICE_PULL_REQUEST_LOG], "cursor": ""}],
           lambda: list(starkbank.invoicepullrequest.log.query(
               limit=5, types=["success"], request_ids=["6234161654976518"])))
    record("invoicepullrequest.log.page", [{"logs": [INVOICE_PULL_REQUEST_LOG], "cursor": ""}],
           lambda: starkbank.invoicepullrequest.log.page(limit=5)[0])

    # payment is polymorphic; transfer and boleto-payment are the two types
    # this build's goldens exercise - see paymentrequest.h.
    payment_request_transfer = starkbank.PaymentRequest(
        center_id="5656565656565656", payment=transfer, tags=["urgent"])
    record("paymentrequest.create", [{"requests": [PAYMENT_REQUEST_TRANSFER]}],
           lambda: starkbank.paymentrequest.create([payment_request_transfer]))
    payment_request_boleto_payment = starkbank.PaymentRequest(
        center_id="5656565656565656", payment=boleto_payment, tags=["urgent"])
    record("paymentrequest.create.boletopayment",
           [{"requests": [PAYMENT_REQUEST_BOLETO_PAYMENT]}],
           lambda: starkbank.paymentrequest.create([payment_request_boleto_payment]))
    record("paymentrequest.query", [{"requests": [PAYMENT_REQUEST_TRANSFER,
                                                  PAYMENT_REQUEST_BOLETO_PAYMENT],
                                     "cursor": ""}],
           lambda: list(starkbank.paymentrequest.query(center_id="5656565656565656", limit=5)))
    record("paymentrequest.page", [{"requests": [PAYMENT_REQUEST_TRANSFER,
                                                 PAYMENT_REQUEST_BOLETO_PAYMENT],
                                    "cursor": ""}],
           lambda: starkbank.paymentrequest.page(center_id="5656565656565656", limit=5)[0])

    account = starkbank.VerifiedAccount(
        tax_id="20.018.183/0001-80",
        bank_code="20018183",
        branch_code="1357-9",
        key_id="tony@starkbank.com",
        name="Anthony Edward Stark",
        number="876543-2",
        type="checking",
        tags=["employees", "monthly"],
    )
    record("verifiedaccount.create", [{"accounts": [VERIFIED_ACCOUNT]}],
           lambda: starkbank.verifiedaccount.create([account]))
    record("verifiedaccount.get", [{"account": VERIFIED_ACCOUNT}],
           lambda: starkbank.verifiedaccount.get("6155165527080960"))
    record("verifiedaccount.delete", [{"account": VERIFIED_ACCOUNT}],
           lambda: starkbank.verifiedaccount.cancel("6155165527080960"))
    record("verifiedaccount.query", [{"accounts": [VERIFIED_ACCOUNT], "cursor": ""}],
           lambda: list(starkbank.verifiedaccount.query(limit=5, status="active",
                                                        tags=["employees"])))
    record("verifiedaccount.page", [{"accounts": [VERIFIED_ACCOUNT], "cursor": ""}],
           lambda: starkbank.verifiedaccount.page(limit=5)[0])
    record("verifiedaccount.log.get", [{"log": VERIFIED_ACCOUNT_LOG}],
           lambda: starkbank.verifiedaccount.log.get("6341320293482502"))
    record("verifiedaccount.log.query", [{"logs": [VERIFIED_ACCOUNT_LOG], "cursor": ""}],
           lambda: list(starkbank.verifiedaccount.log.query(
               limit=5, types=["active"], account_ids=["6155165527080960"])))
    record("verifiedaccount.log.page", [{"logs": [VERIFIED_ACCOUNT_LOG], "cursor": ""}],
           lambda: starkbank.verifiedaccount.log.page(limit=5)[0])

    transfer2 = starkbank.VerifiedTransfer(
        amount=1234,
        account_id="6155165527080960",
        external_id="my-internal-id-654321",
        description="Payment for service #1234",
        display_description="Sword sharpening",
        tags=["arya", "stark"],
        rules=[starkbank.transfer.Rule(key="resendingLimit", value=5)],
    )
    record("verifiedtransfer.create", [{"transfers": [VERIFIED_TRANSFER]}],
           lambda: starkbank.verifiedtransfer.create([transfer2]))

    receiver = starkbank.SplitReceiver(
        name="Anthony Edward Stark",
        tax_id="20.018.183/0001-80",
        bank_code="20018183",
        branch_code="1357-9",
        account_number="876543-2",
        account_type="checking",
        tags=["seller/123456"],
    )
    record("splitreceiver.create", [{"receivers": [SPLIT_RECEIVER]}],
           lambda: starkbank.splitreceiver.create([receiver]))
    record("splitreceiver.get", [{"receiver": SPLIT_RECEIVER}],
           lambda: starkbank.splitreceiver.get("6155165527080962"))
    record("splitreceiver.query", [{"receivers": [SPLIT_RECEIVER], "cursor": ""}],
           lambda: list(starkbank.splitreceiver.query(
               limit=5, status="success", tax_id="20.018.183/0001-80")))
    record("splitreceiver.page", [{"receivers": [SPLIT_RECEIVER], "cursor": ""}],
           lambda: starkbank.splitreceiver.page(limit=5)[0])
    record("splitreceiver.log.get", [{"log": SPLIT_RECEIVER_LOG}],
           lambda: starkbank.splitreceiver.log.get("6341320293482503"))
    record("splitreceiver.log.query", [{"logs": [SPLIT_RECEIVER_LOG], "cursor": ""}],
           lambda: list(starkbank.splitreceiver.log.query(
               limit=5, types=["success"], receiver_ids=["6155165527080962"])))
    record("splitreceiver.log.page", [{"logs": [SPLIT_RECEIVER_LOG], "cursor": ""}],
           lambda: starkbank.splitreceiver.log.page(limit=5)[0])

    profile = starkbank.SplitProfile(
        delay=604800,
        interval="week",
        tags=["default"],
    )
    record("splitprofile.put", [{"profiles": [SPLIT_PROFILE]}],
           lambda: starkbank.splitprofile.put([profile]))
    record("splitprofile.get", [{"profile": SPLIT_PROFILE}],
           lambda: starkbank.splitprofile.get("6155165527080963"))
    record("splitprofile.query", [{"profiles": [SPLIT_PROFILE], "cursor": ""}],
           lambda: list(starkbank.splitprofile.query(limit=5)))
    record("splitprofile.page", [{"profiles": [SPLIT_PROFILE], "cursor": ""}],
           lambda: starkbank.splitprofile.page(
               limit=5, receiver_ids=["6155165527080962"])[0])
    record("splitprofile.log.get", [{"log": SPLIT_PROFILE_LOG}],
           lambda: starkbank.splitprofile.log.get("6341320293482504"))
    record("splitprofile.log.query", [{"logs": [SPLIT_PROFILE_LOG], "cursor": ""}],
           lambda: list(starkbank.splitprofile.log.query(
               limit=5, types=["created"], profile_ids=["6155165527080963"])))
    record("splitprofile.log.page", [{"logs": [SPLIT_PROFILE_LOG], "cursor": ""}],
           lambda: starkbank.splitprofile.log.page(limit=5)[0])

    record("split.get", [{"split": SPLIT}],
           lambda: starkbank.split.get("6155165527080964"))
    record("split.query", [{"splits": [SPLIT], "cursor": ""}],
           lambda: list(starkbank.split.query(
               limit=5, status="success", receiver_ids=["5706627130851328"])))
    record("split.page", [{"splits": [SPLIT], "cursor": ""}],
           lambda: starkbank.split.page(limit=5)[0])

    record("split.log.get", [{"log": SPLIT_LOG}],
           lambda: starkbank.split.log.get("6341320293482505"))
    record("split.log.query", [{"logs": [SPLIT_LOG], "cursor": ""}],
           lambda: list(starkbank.split.log.query(
               limit=5, types=["success"], split_ids=["6155165527080964"])))
    record("split.log.page", [{"logs": [SPLIT_LOG], "cursor": ""}],
           lambda: starkbank.split.log.page(limit=5)[0])

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
            "deposit": {"deposit": DEPOSIT},
            "deposits": {"deposits": [DEPOSIT], "cursor": ""},
            "depositLog": {"log": DEPOSIT_LOG},
            "depositLogs": {"logs": [DEPOSIT_LOG], "cursor": ""},
            "key": {"key": DICT_KEY},
            "keys": {"keys": [DICT_KEY], "cursor": ""},
            "institutions": {"institutions": [INSTITUTION], "cursor": ""},
            "transaction": {"transaction": TRANSACTION},
            "transactions": {"transactions": [TRANSACTION], "cursor": ""},
            "workspace": {"workspace": WORKSPACE},
            "workspaces": {"workspaces": [WORKSPACE], "cursor": ""},
            "corporateHolder": {"holder": CORPORATE_HOLDER},
            "corporateHolders": {"holders": [CORPORATE_HOLDER], "cursor": ""},
            "corporateHolderLog": {"log": CORPORATE_HOLDER_LOG},
            "corporateBalances": {"balances": [CORPORATE_BALANCE], "cursor": ""},
            "corporateCard": {"card": CORPORATE_CARD},
            "corporateCards": {"cards": [CORPORATE_CARD], "cursor": ""},
            "corporateCardLog": {"log": CORPORATE_CARD_LOG},
            "corporatePurchase": {"purchase": CORPORATE_PURCHASE},
            "corporatePurchases": {"purchases": [CORPORATE_PURCHASE], "cursor": ""},
            "corporatePurchaseLog": {"log": CORPORATE_PURCHASE_LOG},
            "corporateInvoice": {"invoice": CORPORATE_INVOICE},
            "corporateInvoices": {"invoices": [CORPORATE_INVOICE], "cursor": ""},
            "corporateTransaction": {"transaction": CORPORATE_TRANSACTION},
            "corporateTransactions": {"transactions": [CORPORATE_TRANSACTION], "cursor": ""},
            "corporateWithdrawal": {"withdrawal": CORPORATE_WITHDRAWAL},
            "corporateWithdrawals": {"withdrawals": [CORPORATE_WITHDRAWAL], "cursor": ""},
            "merchantSession": {"session": MERCHANT_SESSION},
            "merchantSessions": {"sessions": [MERCHANT_SESSION], "cursor": ""},
            "merchantSessionLog": {"log": MERCHANT_SESSION_LOG},
            "sessionPurchase": {"purchase": SESSION_PURCHASE},
            "merchantCard": {"card": MERCHANT_CARD},
            "merchantCards": {"cards": [MERCHANT_CARD], "cursor": ""},
            "merchantCardLog": {"log": MERCHANT_CARD_LOG},
            "merchantInstallment": {"installment": MERCHANT_INSTALLMENT},
            "merchantInstallments": {"installments": [MERCHANT_INSTALLMENT], "cursor": ""},
            "merchantInstallmentLog": {"log": MERCHANT_INSTALLMENT_LOG},
            "merchantPurchase": {"purchase": MERCHANT_PURCHASE},
            "merchantPurchases": {"purchases": [MERCHANT_PURCHASE], "cursor": ""},
            "merchantPurchaseLog": {"log": MERCHANT_PURCHASE_LOG},
            "cardMethods": {"methods": [CARD_METHOD], "cursor": ""},
            "merchantCategories": {"categories": [MERCHANT_CATEGORY], "cursor": ""},
            "merchantCountries": {"countries": [MERCHANT_COUNTRY], "cursor": ""},
            "boletoHolmes": {"holmes": BOLETO_HOLMES},
            "boletoHolmesList": {"holmes": [BOLETO_HOLMES], "cursor": ""},
            "boletoHolmesLog": {"log": BOLETO_HOLMES_LOG},
            "dynamicBrcode": {"brcode": DYNAMIC_BRCODE},
            "dynamicBrcodes": {"brcodes": [DYNAMIC_BRCODE], "cursor": ""},
            "invoicePullSubscription": {"subscription": INVOICE_PULL_SUBSCRIPTION},
            "invoicePullSubscriptions": {"subscriptions": [INVOICE_PULL_SUBSCRIPTION],
                                         "cursor": ""},
            "invoicePullSubscriptionLog": {"log": INVOICE_PULL_SUBSCRIPTION_LOG},
            "invoicePullRequest": {"request": INVOICE_PULL_REQUEST},
            "invoicePullRequests": {"requests": [INVOICE_PULL_REQUEST], "cursor": ""},
            "invoicePullRequestLog": {"log": INVOICE_PULL_REQUEST_LOG},
            "paymentRequest": {"request": PAYMENT_REQUEST_TRANSFER},
            "paymentRequestTransfer": {"requests": [PAYMENT_REQUEST_TRANSFER]},
            "paymentRequestBoletoPayment": {"requests": [PAYMENT_REQUEST_BOLETO_PAYMENT]},
            "paymentRequests": {"requests": [PAYMENT_REQUEST_TRANSFER,
                                             PAYMENT_REQUEST_BOLETO_PAYMENT], "cursor": ""},
            "verifiedAccount": {"account": VERIFIED_ACCOUNT},
            "verifiedAccounts": {"accounts": [VERIFIED_ACCOUNT], "cursor": ""},
            "verifiedAccountLog": {"log": VERIFIED_ACCOUNT_LOG},
            "verifiedTransfers": {"transfers": [VERIFIED_TRANSFER]},
            "splitReceiver": {"receiver": SPLIT_RECEIVER},
            "splitReceivers": {"receivers": [SPLIT_RECEIVER], "cursor": ""},
            "splitReceiverLog": {"log": SPLIT_RECEIVER_LOG},
            "splitProfile": {"profile": SPLIT_PROFILE},
            "splitProfiles": {"profiles": [SPLIT_PROFILE], "cursor": ""},
            "splitProfileLog": {"log": SPLIT_PROFILE_LOG},
            "splitLog": {"log": SPLIT_LOG},
            "split": {"split": SPLIT},
            "splits": {"splits": [SPLIT], "cursor": ""},
        },
    }
    with open(OUT, "w") as handle:
        json.dump(document, handle, indent=2, sort_keys=True)
        handle.write("\n")
    print("recorded {count} cases into {path}".format(count=len(CASES), path=OUT))


if __name__ == "__main__":
    main()
