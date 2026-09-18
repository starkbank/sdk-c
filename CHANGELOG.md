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
- Windows is compile-only in CI and has no runner that links: core-c has no
  Windows build yet. The DLL path is the least-tested surface here.
