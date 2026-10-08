# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to the following versioning pattern:

Given a version number MAJOR.MINOR.PATCH, increment:

- MAJOR version when **breaking changes** are introduced;
- MINOR version when **backwards compatible changes** are introduced;
- PATCH version when backwards compatible bug **fixes** are implemented.

`STARKBANK_ABI_VERSION` is separate and moves only when the meaning of a
handle, a return code or a prototype changes. Adding a field or a resource
never bumps it: fields are not part of the ABI, which is the point of the
name-keyed accessors.

## [Unreleased]
### Added
- InvoicePullSubscription (+ Log): `cancel` keeps sdk-c's uniform `*_delete`
  spelling. `due`/`end` are STRING, not DATE_OR_DATETIME: sdk-python's own
  conditional-expression coercion is invisible to `tools/drift.py`'s ast
  reader. `Log.errors` is LIST_OBJECT, not the docstring's "list of strings":
  the api-v2-ms-invoice-pull service answers with `{code, message}` objects,
  the same docstring-says-strings-but-sends-objects-uncoerced gap
  MerchantCard/MerchantInstallment/MerchantPurchase.Log's errors already carry
- InvoicePullRequest (+ Log): same `cancel`-as-`delete` spelling and the same
  LIST_OBJECT `Log.errors`; `due` is DATE_OR_DATETIME here, a direct
  `check_datetime_or_date` call with no exemption needed
- BoletoHolmes (+ Log): no errors field and no pdf verb, either - sdk-python's
  Log has neither
- DynamicBrcode (+ Rule): `get` reaches `GET /v2/dynamic-brcode/:uuid`, not
  `:id`, the same `:uuid`/`:id` placeholder gap the docs show between
  `POST /v2/merchant-session/:uuid/purchase` and `GET /v2/merchant-session/:id`
- PaymentRequest: the first CREATE-writable polymorphic RESOURCE field in
  this SDK. `payment` hydrates as whichever of `Transfer`, `Transaction`,
  `BoletoPayment`, `BrcodePayment`, `UtilityPayment`, `DarfPayment` or
  `TaxPayment` its sibling `type` names - the same `STARKBANK_POLYMORPH`
  mechanism `Event.log` and `PaymentPreview.payment` already use - and is
  written with the existing `starkbank_entity_set_json_raw` escape hatch
  rather than a new setter, the same choice `MerchantSession`'s
  `Purchase.metadata` already made for a single CREATE-writable object field.
  `due` is STRING for the same reason as InvoicePullSubscription's `due`/`end`
### Added
- `STARKBANK_VERB_PUT_MULTI`: `rest.put_multi`, structurally identical to
  `POST_MULTI` (the same `{plural: [...]}` envelope, the same REQUIRED
  pre-flight, list in and list out) but for the HTTP verb, built on core-c's
  `starkcore_rest_put_multi` - already exposed and unused by this SDK until
  now, so no core-c change was needed, the same precedent
  `STARKBANK_VERB_POST_SUB_RESOURCE` set for `MerchantSession.purchase`
- VerifiedAccount (+ Log): `create`/`get`/`query`/`page` plus `delete`, which
  keeps sdk-c's uniform `starkbank_verified_account_delete` spelling for
  python's `cancel()` - the same choice already made for CorporateHolder.
  `verifiedaccount.Log.errors` is a real `LIST_OBJECT`, despite sdk-python's
  own docstring calling the field a list of strings: app-docs'
  `verified-account.js` sample shows `"errors": [{"code": "keyNotFound",
  "message": "The key is not registered"}]` on the wire, and the
  api-v2-ms-transfer service that backs this resource confirms it in code -
  `models/verifiedAccountLog.py`'s `errorDescriptions` (lines 37-42) builds
  exactly that `{code, message}` shape, and python's own `__init__` does no
  coercion either way to contradict it
- VerifiedTransfer: `create` only, sdk-python's module exports no
  `get`/`query`/`page` at all. `rules` reuses the existing `Transfer.Rule`
  table rather than a local one - sdk-python's own module imports
  `transfer.rule.Rule` directly and hydrates through its `_sub_resource`
- SplitReceiver (+ Log): the bank account a `Split.receiverId` names. The
  query key list is sdk-python's `query()` forwarded set, since `QUERY` and
  `PAGE` share one table and `page()` is only a narrower subset of it; this
  disagrees with the docs' `GET /v2/split-receiver` parameter list in five
  places (a `receiverIds` filter no SplitReceiver field or python keyword
  names, a `taxIds`/`taxId` plural/singular mismatch, and `sort`/
  `transactionIds` the docs omit), each recorded in `known-drift.json`.
  `splitreceiver.Log.errors` is `LIST_STRING`: the api-v2-ms-split service
  never emits the key
- SplitProfile (+ Log): the first consumer of `STARKBANK_VERB_PUT_MULTI` -
  sdk-python's only write verb is `put(splitProfile)`, with no create and no
  delete. `delay` and `interval` are REQUIRED despite the class docstring
  filing them under "optional": `__init__` takes both positionally with no
  default, so the docstring's wording is the bug, recorded as two
  `flag.conflict` entries resolved to "table". `delay` is `NUMBER`, not
  `SECONDS`, the same reasoning as `MerchantSession.expiration`. The query key
  list is `limit`, `after`, `before`, `ids`, `receiverIds`, `status`, `tags`:
  `receiverIds` is a real keyword `page()` forwards to `rest.get_page`, even
  though the docs' `GET /v2/split-profile` parameter list omits it, recorded
  as `query.gone:SplitProfile:receiverIds` in `known-drift.json`
- Split (+ Log): `get`/`query`/`page` now arrive alongside the fields Invoice
  already hydrated out of its `splits` list. The query key list - `limit`,
  `after`, `before`, `ids`, `receiverIds`, `status`, `tags` - is sdk-python's
  `query()`/`page()` forwarded set, byte for byte the docs' `GET /v2/split`
  parameter list too, so no `known-drift.json` entry is needed; the three
  `verb.new:Split:get/query/page` exemptions that used to hide the gap are
  removed. `split.Log`'s own `get`/`query`/`page` over `split/log` stays
  reachable independently, as before. `errors` on both Logs is `LIST_STRING` -
  the api-v2-ms-split service never emits the key, unlike VerifiedAccount's
  api-v2-ms-transfer

### Removed
- `starkbank_merchant_session_purchase`: sdk-python's
  `merchantsession.purchase()` is now deprecated and always raises, because
  `POST /v2/merchant-session/:uuid/purchase` carries raw card data and must
  be called from the payer's front-end, never from the merchant's back-end.
  sdk-c drops the verb rather than keep a working one python refuses to
  offer - the same call as Transaction's missing `create`. The `Purchase`
  table stays, since sdk-python still exports the class, and
  `STARKBANK_VERB_POST_SUB_RESOURCE` stays in the engine with no table using it

### Fixed
- `starkbankVerbCreate` and `starkbankVerbPutMulti` (`starkc/verb.c`) now free
  the dehydrated `item` when `starkcore_json_append(payload, item)` fails:
  `payload` never took ownership on a failed append, so `item` leaked on that
  path in both functions - `PutMulti` was copied from `Create` and carried
  the same bug forward

## [0.1.0] - 2026-09-18
### Added
- Public header `include/starkbank.h`: the whole ABI in one C89-clean file -
  opaque handles, 28 generic accessors, the list and iterator API, the
  reflected registry, the client and user lifecycle, the transport seam, and
  `starkbank_parse_and_verify`
- `starkc/`: the table-driven engine - entity, table, list, iter, verbs, verb,
  registry, error and facade. The client owns the user and checks
  `starkcore_abi_version()`; query and patch bags are tagged entities, so a
  filter typo is caught locally instead of by the API
- DarfPayment (+ Log): fully structured, unlike UtilityPayment and TaxPayment
  - no conditionally-required `line`/`barCode` pair, and no `type`
  attribute, because sdk-python's DarfPayment carries none
- First resource slice: Invoice (+ Log, Payment, Rule, Split), Transfer
  (+ Log, Rule), Event (+ Attempt) and Balance, each one field table plus
  `STARKBANK_VERB_*` lines
- TaxPayment (+ Log): `line` and `barCode` are sdk-python's
  conditionally-required pair, exactly as on UtilityPayment and on
  BoletoPayment; `scheduled` is a plain DATE, not DATE_OR_DATETIME
- UtilityPayment (+ Log): the same conditionally-required `line`/`barCode`
  pair as TaxPayment; `scheduled` is a plain DATE
- Webhook and PaymentPreview (+ BrcodePreview, BoletoPreview, TaxPreview,
  UtilityPreview), which close the last two `starkcore_rest_*` shapes the bank
  SDK uses. Every resource here is still a table and nothing else: the two new
  shapes are engine work, not per-resource code
- `STARKBANK_VERB_POST_SINGLE`: `rest.post_single`, where the body is the
  entity itself rather than a list. `starkbank_webhook_create` therefore takes
  an entity and borrows it, where every other create takes a list and owns it
- Polymorphic fields are a table, not a function. A resource declares
  `STARKBANK_POLYMORPH(ident, field, discriminator, VARIANTS)` and the engine
  resolves the nested table from a sibling field of the document;
  `PaymentPreview.payment` by `type` and `Event.log` by `subscription` are now
  the same mechanism, and `starkbank/event/event.c` lost the only hand-written
  function any resource had
- `handwritten/event_parse.c`, behind the linker-enforced opt-out: a judgement
  method is declared in the header, defined by no macro, and the link fails
  when nobody writes it
- `tools/drift.py`: six checks - verb, field, endpoint, query, comment and the
  `type.conflict` hard stop - against sdk-python at the pinned sha and the
  app-docs resource JSON, failing on a change to the accepted set in
  `tests/reference/known-drift.json`
- `tools/reflect`: the expanded tables, the endpoints core-c derives and
  core-c's own casing rule, printed as JSON for the Python tools
- `tools/emit.py`: the Delphi unit, the C# P/Invoke class and one compilable
  docs sample per mechanical verb, all byte-deterministic and regenerated in CI
- Build: static, shared with a link-time export list, the single-artifact
  bundle with and without the libcurl transport, `install` and `starkbank.pc`,
  `make expand` for macro debugging and `make print-ldflags` for the static
  link order
- Suites: engine, slice against goldens recorded from sdk-python, the core-ABI
  guard, the loose-query build, ASan/UBSan, leaks/valgrind and a
  ThreadSanitizer job sharing one client across eight threads
- Boleto (+ Log): due is a plain DATE, unlike Invoice's DATE_OR_DATETIME,
  because sdk-python calls `check_date` and there is no scheduled-boleto
  equivalent
- BoletoPayment (+ Log)
- BrcodePayment (+ Log, Rule): `status` is PATCH-only, the same shape as
  `Invoice.status`, because sdk-python's `update()` accepts only that key
- `STARKBANK_VERB_CONTENT_QUERY`: an engine verb shape for a content route
  with two optional string query keys, both omitted when empty - generalises
  `CONTENT_INT`'s "0 means send nothing" rule to `Boleto.pdf`'s `layout` and
  `hiddenFields` (a comma-joined list; core-c encodes a list the same way, so
  the wire bytes match sdk-python's) rather than hand-writing a per-resource
  function
- Deposit (+ Log): no create at all - every field but `amount` is return-only,
  and `amount` reaches the wire only through `update`'s reversal (full or
  partial, `amount=0` fully reverses). `deposit.Log` carries a `pdf` verb none
  of the other logs have
- DictKey: query/get only, no create and no update, exactly as sdk-python has
  none either. `id` is the PIX key itself
- Institution: no id, no create. `page` is python's `query()` under the shape
  it actually is - `rest.get_page(...)[0]`, one call, cursor discarded - so
  sdk-c names the verb for what it does rather than inventing an iterator
  python's own code never builds
- Transaction: read-only end to end. sdk-python's `create()` is deprecated
  since v2.31.0 and unconditionally raises `StarkError`; sdk-c adds no create
  verb rather than letting a C caller do what python refuses to do at all
- Workspace: `create` is `post_single`, like Webhook. `picture` needed no new
  engine shape: sdk-python base64-encodes the bytes itself into one
  `data:<mime>;base64,<...>` wire string, so the existing generic
  `starkbank_entity_set_string` already covers it - the caller builds the same
  string sdk-python builds
- CorporateHolder (+ Log), `CorporateHolder.Permission` (a bare sub-resource):
  `delete` keeps sdk-c's uniform `starkbank_<resource>_delete` spelling even
  though sdk-python calls the same DELETE call `cancel()`
- CorporateRule (+ CorporateBalance): a bare sub-resource embedded in
  `CorporateHolder.rules` and `CorporateCard.rules`, the same shape as
  `Split`; `categories`/`countries`/`methods` are `LIST_OBJECT`, not
  `LIST_RESOURCE`, because `MerchantCategory`, `MerchantCountry` and
  `CardMethod` are not registered resources in this build. CorporateBalance
  is the same degenerate no-id, one-verb shape `Balance` already established
- CorporateCard (+ Log): `create` posts to `corporate-card/token`, which
  needed a new engine verb macro, `STARKBANK_VERB_POST_SINGLE_SUB` -
  `starkbankVerbCreateSub` in `starkc/verb.c`. `pin` is PATCH-only with no
  matching field, the same shape as sdk-c4's `Workspace.picture`
- CorporatePurchase (+ Log): every field is RO - a purchase is authorized by
  the card network, nobody posts one. `errors` on the Log is a real
  `LIST_OBJECT`, unlike CorporateCardLog and CorporateHolderLog, which carry
  none. `parse` and `response` are hand-written in
  `handwritten/corporatepurchase_parse.c` and `_response.c`: `parse` verifies
  with no envelope key, and `response` builds `{"authorization": {...}}` with
  no network call at all
- CorporateTransaction, CorporateInvoice and CorporateWithdrawal:
  CorporateTransaction is a read-only ledger entry with no create, update or
  delete anywhere in sdk-python; CorporateInvoice and CorporateWithdrawal are
  both `post_single`, the same shape as Webhook
- MerchantSession (+ `MerchantSession.AllowedInstallment`, `Purchase`, Log):
  `purchase(uuid, purchase)` needed a new engine verb,
  `STARKBANK_VERB_POST_SUB_RESOURCE` / `starkbankVerbCreateSubResource` in
  `starkc/verb.c`, built on `starkcore_rest_post_sub_resource` - already
  exposed by core-c and unused by this SDK until now, so no core-c change was
  needed. `AllowedInstallment` registers qualified
  (`MerchantSession.AllowedInstallment`), not bare, because sdk-python
  assigns it `_sub_resource`, not `_resource` - `tools/drift.py`'s python
  reader qualifies every `_sub_resource` by its owning package regardless of
  name collision, the same mechanism behind `Invoice.Rule`/`Transfer.Rule`.
  `expiration` is `NUMBER`, not `SECONDS`: sdk-python's `__init__` never
  calls `check_timedelta` on it, despite the docstring's "integer or
  timedelta" wording. `merchantsession.Log.errors` is `LIST_STRING`, served
  by a different backing service than the other three merchant logs
- MerchantCard (+ Log): every field is RO, get/query/page only. `errors` on
  the Log is a real `LIST_OBJECT`, the same shape CorporatePurchase.Log uses
- MerchantInstallment (+ Log): every field is RO, generated automatically
  when a MerchantPurchase is split. `purchaseIds` is a real query()/page()
  filter the docs omit
- MerchantPurchase (+ Log): `create` is `post_single`; `update(status,
  amount)` sends exactly those two keys, both PATCH-only. `metadata` is the
  first CREATE-writable single `OBJECT` field in this SDK - written with the
  existing `starkbank_entity_set_json_raw` escape hatch, no new engine shape
- CardMethod, MerchantCategory and MerchantCountry: query-only lookups for
  `CorporateRule.methods`/`categories`/`countries`, each with sdk-python's
  `query(search=None)` and no `limit` keyword, no `page()`. Promoting
  `CorporateRule`'s matching fields from `LIST_OBJECT` to `LIST_RESOURCE` is
  a mechanical follow-up left for that table, as its own header already flags
- `make dist`: the release tarball for one OS and architecture - header, the
  plain archive, the two bundles static and shared, a relocatable `.pc` and the
  bindings - with a sha256 beside it
- `.github/workflows/release.yml`: on a version tag, builds the tarball on
  Linux x86_64 (Ubuntu 22.04 container, glibc 2.35 floor), macOS arm64, macOS
  x86_64 and Windows x86_64, refuses a tag that disagrees with
  `STARKBANK_VERSION`, and attaches the tarballs to a GitHub Release;
  `workflow_dispatch` rehearses the packaging without publishing
- The Linux and Windows release jobs build libsecp256k1 v0.7.1 from source:
  Ubuntu 22.04's 0.1 package has a `SECP256K1_CONTEXT_NONE` that cannot sign,
  so a build against it passes the compiler and fails every request, and MSYS2
  has no package at all. Every release job runs the suites before packaging
- Windows through MinGW-w64 in MSYS2's UCRT64 environment: the Makefile grows a
  platform branch (`starkbank.dll` and `starkbank_full.dll` with import
  libraries and `.def` files, no `-fPIC`, static libgcc, `-lbcrypt` for
  ecdsa-c's random source on every DLL including the bundle). `check-exports`
  reads the PE export table, ignores GCC's `.refptr.` stubs in the archive and
  requires every shared artifact to export exactly the `STARKBANK_API` count;
  the merged archives are handed to `ar -M` by Windows-style paths, because
  MSYS2 converts none inside a script on stdin. A CI job builds libsecp256k1
  from source with the recovery module, builds ecdsa-c with `SECP256K1_STATIC`
  (the header declares the API `dllimport` without it and the link asks for
  `__imp_secp256k1_*`), pins core-c 0.1.1, whose vendored cJSON no longer
  marks itself `dllexport` (the directive survives `--exclude-libs`, so 0.1.0
  leaked its 78 functions from every DLL), and runs the suites there. No curl
  bundle on Windows. The MSVC compile-only gate now discovers every resource
  directory instead of naming five

### Changed
- `tests/reference/sdk-python.sha` moved to `be7755a5`, the sdk-python master
  the Webhook and PaymentPreview tables and the goldens were read from. The
  two commits it crosses are docstring grammar and CI, and re-recording
  `tests/reference/slice.json` across the bump reproduced every pre-existing
  case byte for byte.

### Notes
- Webhook's create sends the entity as the body, with no envelope key. python
  wraps a create payload under the plural key for `post_multi` only; the
  singular key belongs to the response. The golden carries python's own bytes
  for that request, so the claim is recorded rather than asserted.
- `starkinfra/core-c` and `starkbank/ecdsa-c` are required and are not
  vendored. Every CI job is red until both are reachable from this repository.
- Windows links and runs the suites in CI through MinGW-w64 (the
  `windows-mingw` job), and every release job runs them before packaging. MSVC
  stays compile-only: core-c has no MSVC build to link.

### Fixed
- The curl bundles carried no curl transport. `starkbank_client_set_curl_transport`
  is compiled under `STARKBANK_WITH_CURL`, which no bundle target defined, so
  the facade never referenced `starkcore_transport_curl`, the linker dropped
  libstarkcore_curl.a's only member, and both curl bundles were the plain ones
  under another name. The curl bundles now compile the facade with the define
  (a second object, so the plain bundle keeps answering NO_TRANSPORT), and
  `check-exports` fails unless the curl bundle carries the transport and links
  libcurl and the plain one does neither
- `make shared` built the dylib from a phony target, so `-install_name
  @rpath/$(@F)` stamped it `@rpath/shared` and no host loading it by rpath
  found it. The shared library is a file target now
