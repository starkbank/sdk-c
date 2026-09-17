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
