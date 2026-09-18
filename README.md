## starkbank for C

### Overview

The Stark Bank resource tier in C, over `starkcore` and `ecdsa-c`: invoices,
transfers, events, webhooks, payment previews and the rest of the API surface
as tables the preprocessor expands against one hand-written engine.

It re-implements nothing that `core-c` owns. Casing, endpoint derivation,
envelope keys, pagination arithmetic, request signing and status mapping are
called at run time, never copied - `starkcore_api_endpoint("InvoiceLog")` is
what turns a resource name into `/v2/invoice/log`. The behavioural reference
above that is `sdk-python`, and the suite checks this library against goldens
recorded by running it.

There is no HTTP and no TLS here either. The host supplies a transport through
`starkbank_client_set_transport`, or links core-c's optional libcurl one.

### The ABI, in one sentence

Four handle types, 28 accessors keyed by wire-field name, and one entry point
per verb - so **fields are not part of the ABI**, and a new API field is one
table line with no new symbol, no soname bump and no host recompile.

```c
starkbank_invoice_new(&invoice);
starkbank_entity_set_amount(invoice, STARKBANK_INVOICE_AMOUNT, 400000);
starkbank_entity_set_string(invoice, STARKBANK_INVOICE_TAX_ID, "012.345.678-90");
starkbank_entity_set_string(invoice, STARKBANK_INVOICE_NAME, "Arya Stark");
starkbank_list_new(&batch);
starkbank_list_append(batch, invoice);          /* the list owns it from here */
status = starkbank_invoice_create(client, batch, &created, &errors);
```

`include/starkbank.h` is the whole contract: the ownership rules, the threading
rules, the field tables per resource and what each return code means. It is
C89-clean, uses no `long`, no `bool` and no stdint, and every declaration
carries `STARKBANK_API` and `STARKBANK_CALL` - so a Delphi header translator,
an FFI generator and MSVC can all read it. `make check-header` proves that.

### Resources in this build

| resource | verbs | notes |
|---|---|---|
| CardMethod | query | query-only lookup for `CorporateRule.methods`; no `limit` keyword and no `page`, matching python's `query(search=None)` exactly |
| CorporateBalance | get | no id, no filters: the degenerate shape, again |
| CorporateCard | create get query page update delete | `create` posts to `corporate-card/token` via `STARKBANK_VERB_POST_SINGLE_SUB`. + `corporatecard.Log` (get query page). `pin` is PATCH-only with no matching field: python's `update()` sends it but never stores it |
| CorporateHolder | create get query page update delete | + `corporateholder.Log` (get query page), `CorporateHolder.Permission` (bare sub-resource), `CorporateRule` |
| CorporateInvoice | create query page | `create` is `post_single`, the Webhook shape; no `get` - python has none |
| CorporatePurchase | get query page parse response | every field is RO: a network authorizes a purchase, nobody posts one. + `corporatepurchase.Log` (get query page), whose `errors` is a real LIST_OBJECT. `parse`/`response` are hand-written, no network call in `response` |
| CorporateRule | (none) | bare sub-resource embedded in `CorporateHolder.rules` and `CorporateCard.rules`, the same shape as `Split` |
| CorporateTransaction | get query page | a read-only ledger entry; no create, update or delete anywhere in python |
| CorporateWithdrawal | create get query page | `create` is `post_single`, the same shape as `CorporateInvoice` |
| MerchantCard | get query page | every field is RO: stored once a MerchantSession Purchase or MerchantPurchase succeeds, never posted. + `merchantcard.Log` (get query page), whose `errors` is a real LIST_OBJECT |
| MerchantCategory | query | query-only lookup for `CorporateRule.categories`; no `limit` keyword and no `page` |
| MerchantCountry | query | query-only lookup for `CorporateRule.countries`; no `limit` keyword and no `page` |
| MerchantInstallment | get query page | every field is RO: generated automatically when a MerchantPurchase is split. + `merchantinstallment.Log` (get query page), whose `errors` is a real LIST_OBJECT |
| MerchantPurchase | create get query page update | `create` is `post_single`. `update` sends only `status`/`amount`, to cancel an approved purchase or reverse a confirmed one. + `merchantpurchase.Log` (get query page), whose `errors` is a real LIST_OBJECT |
| MerchantSession | create get query page purchase | `create` is `post_single`. `purchase` is new: `STARKBANK_VERB_POST_SUB_RESOURCE`, built on `starkcore_rest_post_sub_resource`, POSTs a `Purchase` to `merchant-session/<uuid>/purchase` and returns it with an id. + `MerchantSession.AllowedInstallment`, `Purchase`, `merchantsession.Log` (get query page), whose `errors` is LIST_STRING, unlike the other merchant logs |
| DarfPayment | create get delete query page pdf | + `darfpayment.Log` (get query page). Fully structured: no conditionally-required line/barCode pair, and no `type` attribute |
| Invoice | create get query page update pdf qrcode payment | + `invoice.Log` (get query page pdf), `Invoice.Rule`, `Invoice.Payment`, `Split` |
| TaxPayment | create get delete query page pdf | + `taxpayment.Log` (get query page). `line`/`barCode` are the conditionally-required pair; `scheduled` is a plain DATE |
| Transfer | create get delete query page pdf | + `transfer.Log` (get query page), `Transfer.Rule` |
| Boleto | create get delete query page pdf | + `boleto.Log` (get query page). `pdf` takes an optional `layout` and an optional comma-joined `hiddenFields` string, both sent only when set |
| BoletoPayment | create get delete query page pdf | + `boletopayment.Log` (get query page) |
| BrcodePayment | create get query page update pdf | + `brcodepayment.Log` (get query page), `BrcodePayment.Rule`. `update` is status-only, to cancel before payment |
| Deposit | get query page update | + `deposit.Log` (get query page pdf). No create: passive cash-in only. `update` is amount-only, to fully or partially reverse |
| DictKey | get query page | No create, no update: query/get only, mirroring sdk-python exactly |
| Event | get query page update delete parse | `log` is polymorphic: the table comes from `subscription` |
| Institution | page | No id, no create: `page` is python's `query()` under the engine shape it actually is - one page call, cursor discarded |
| Balance | get | no id: the head of the listing endpoint |
| UtilityPayment | create get delete query page pdf | + `utilitypayment.Log` (get query page). Same conditionally-required `line`/`barCode` pair as TaxPayment; `scheduled` is a plain DATE |
| Transaction | get query page | No create: sdk-python's `create()` is deprecated and always raises |
| Webhook | create get query page delete | `create` is `post_single` and takes one entity, not a list |
| Workspace | create get query page update | `create` is `post_single`, like Webhook. `picture` is one wire string: the caller base64-encodes the bytes into the same `data:<mime>;base64,<...>` string sdk-python builds |
| PaymentPreview | create | `payment` is polymorphic: the table comes from `type`, into `BrcodePreview`, `BoletoPreview`, `TaxPreview` or `UtilityPreview` |

Those six between them use every `starkcore_rest_*` shape the bank SDK needs:
`post_multi`, `post_single`, `get_id`, `get_page`, the stream, `patch_id`,
`delete_id`, `get_content` and `get_sub_resource`. The remaining bank
resources are tables on top of exactly this engine.

Two fields in the surface are polymorphic, and neither costs a line of C. A
resource declares the map beside its field table -

```c
#define STARKBANK_PAYMENT_PREVIEW_VARIANTS(V)             \
    V("brcode-payment",  "PaymentPreview.BrcodePreview")  \
    V("boleto-payment",  "PaymentPreview.BoletoPreview")  \
    ...

STARKBANK_POLYMORPH(payment_preview, "payment", "type",
                    STARKBANK_PAYMENT_PREVIEW_VARIANTS);
```

- and the engine reads the discriminator out of the document being hydrated. A
value the map does not carry leaves the nested object untagged, permissive and
counted as exactly one unknown, so a variant invented after this build ships
is still readable and still shows up in CI.

### Layout

```
include/starkbank.h       the ABI, hand-authored, permanently
starkc/                   the engine: entity table list iter verb verbs.h
                          registry error facade
starkbank/<resource>/     one field table plus STARKBANK_VERB_* lines each
handwritten/              the judgement methods, behind the linker-enforced
                          opt-out: declared in the header, defined by no macro
samples/                  one compilable program per mechanical endpoint
bindings/                 the Delphi unit and the C# P/Invoke class, shipped
tools/                    drift.py, emit.py, reflect.c and the header gate
tests/                    the suites, the goldens and the recorder
```

Adding a resource is a directory, a table, a `.c` of about a dozen lines, a
header block, a registry line and a `SOURCES` line. Adding a field is one line.

### Build

```
make                 libstarkbank.a
make shared          libstarkbank.dylib | libstarkbank.so.1, only starkbank_*
make bundle          one artifact with core-c, ecdsa-c and secp256k1 inside
make bundle-curl     the same, with core-c's libcurl transport
make install PREFIX=/usr/local
make print-ldflags   the static link order, so nobody guesses it
```

`CORE_PREFIX`, `ECDSA_PREFIX` and `SECP256K1_PREFIX` are discovered the way
core-c discovers its own: sibling checkout, Homebrew, `/usr/local`. Nothing is
vendored, and `REQUIRED_CFLAGS` is kept apart from `CFLAGS` so a command-line
override cannot drop `-fvisibility=hidden` and widen the exported surface.

Two consumption shapes, because the consumers differ:

- **C, C++ and POS.** Four archives, no HTTP and no TLS unless the host links
  `libstarkcore_curl.a`. One `.c` per resource plus `-ffunction-sections
  -Wl,--gc-sections` means a terminal that only creates invoices links roughly
  one resource's worth of code.
- **Delphi, .NET, single file.** `make bundle`: one library, one `DllImport`
  name, one allocator, exporting only `starkbank_*`. The bindings in
  `bindings/` are shipped and versioned with it.

### Test

Offline and deterministic. The seam is a fake `starkcore_transport_fn`; core-c
itself is never mocked and nothing reaches the network.

```
make test            the engine and slice suites
make test-loose      the same, built with -DSTARKBANK_LOOSE_QUERY
make test-abi        the core-ABI guard, with starkcore_abi_version stubbed
make asan            AddressSanitizer and UBSan
make leaks           leaks (Darwin) or valgrind (Linux)
make tsan            one client, eight threads
make samples         every docs sample, compiled with -Werror
make check-exports check-handwritten check-header check-drift check-tools
make expand R=invoice   preprocessed output for one resource
```

The goldens under `tests/reference/` are recorded from `sdk-python` by
`tests/tools/record_fixtures.py`, which stubs `requests` before importing it -
no network, no pip install - so every expected byte came from the normative
implementation rather than from someone's belief about it. They are
regenerated by the tool and never hand-edited.

### Drift

The field tables are derived from `sdk-python` and are never a source of
truth. `make check-drift` keeps that true: it reads sdk-python with `ast`
(never importing it) at the sha `tests/reference/sdk-python.sha` pins, reads
the app-docs resource JSON for endpoints and filter names, and compares both
with the expanded tables and with the header's field comment blocks.

It fails when the **set** of accepted divergences in
`tests/reference/known-drift.json` changes - a new finding, or an entry that
no longer fires - and never merely because the set is non-empty. Every entry
carries an owner and a reason. A `type.conflict`, where the docs, python and
the table disagree about a type, is a hard stop that an ordinary exemption
cannot silence: it needs a recorded resolution naming which input won.

### Requirements

`starkinfra/core-c`, `starkbank/ecdsa-c`, `libsecp256k1`, a C99 compiler. The
tools need Python 3 and nothing from PyPI.
