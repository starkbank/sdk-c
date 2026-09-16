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
- First resource slice: Invoice (+ Log, Payment, Rule, Split), Transfer
  (+ Log, Rule), Event (+ Attempt) and Balance, each one field table plus
  `STARKBANK_VERB_*` lines
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

### Notes
- `starkinfra/core-c` and `starkbank/ecdsa-c` are required and are not
  vendored. Every CI job is red until both are reachable from this repository.
- Windows is compile-only in CI and has no runner that links: core-c has no
  Windows build yet. The DLL path is the least-tested surface here.
