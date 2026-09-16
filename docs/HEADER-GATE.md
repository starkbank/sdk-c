# Header gate

`include/starkbank.h` is the ABI. Design risk 2 is that a hand-maintained
header of this size does not parse in the tools an FFI host brings, and the
design calls for that to be falsified in week one, before the engine exists.
This is the record of what was actually run and what is still an assumption.

Run it with `make check-header`.

## What the gate does

| # | Check | How |
|---|---|---|
| 1 | House rules | `tools/houserules.py` |
| 2 | C89, `-pedantic-errors` | `cc -std=c89 -pedantic-errors -Werror -Wall -Wextra -Wshadow -Wpointer-arith -Wwrite-strings -Wcast-qual -fsyntax-only tests/header/c89.c` |
| 3 | C++98 | `c++ -x c++ -std=c++98 -pedantic-errors -Werror … -fsyntax-only tests/header/cxx98.cpp` |
| 4 | cffi / pycparser (opt-in) | `tools/parse_cffi.py`, `ffi.cdef()` over the header with the two calling-convention macros stripped and the starkcore handles forward-declared |
| 5 | Tooling negative cases | `tests/header/test_headerabi.py` |
| 6 | Bindings are reproducible | `tools/emit.py --check` |

Checks 2 and 3 are syntax-only: there is no library to link against yet, and
the gate is about the header, not about symbols.

## cffi is the one dependency, and it is gate-only and opt-in

Every other step here runs on the Python 3 standard library. Check 4 needs
`cffi`, which has to be installed, and installing it means reaching PyPI — so
the gate never does that on its own. It uses `cffi` if it is importable, then a
venv at `$STARKBANK_GATE_VENV` (default `/tmp/starkbank-sdk-c-gate-venv`) if one
already exists, and otherwise skips with

```
gate: cffi step skipped (cffi not importable; set STARKBANK_GATE_CFFI=1 to create ...)
```

Setting `STARKBANK_GATE_CFFI=1` is what authorises creating that venv and
installing `cffi` into it — never into the user's environment. CI's "Public
header gate" step sets it, so the machine-readability evidence still
accumulates where a network is expected; an offline or air-gapped build gets
the sentence above and a green gate on the other five checks.

`cffi` is not a build dependency and not a runtime dependency. Nothing that
ships in `libstarkbank.a` knows it exists.

`-Werror` is load-bearing and was added after the gate was caught passing a
deliberately broken header. clang reports a `//` comment under `-std=c89
-pedantic-errors` as a warning, not an error, so without `-Werror` the dialect
gate had nothing to say about a C99 header.

Check 1 exists because the compiler cannot enforce our rules, only the
language's. `long` is valid C89 everywhere and is 32 bits on Win64 and 64 bits
on LP64, so it truncates an amount on exactly one of a caller's two platforms
and no compiler will mention it. `tools/houserules.py` checks nine such rules
against the comment-stripped header — no `long`, `bool`, stdint types,
`inline`, `restrict`, variadics, visible structs or enums, and no `//` comment
outside a comment — and each hit prints the reason, because a rule nobody
understands is a rule someone deletes. All nine were verified to fire by
injecting a violation into a copy of the header.

Check 3 counts. `ffi.cdef()` raising is the obvious failure, but a stripper
that quietly dropped declarations would make the gate pass by parsing less, so
every name the header declares is looked back up in the parser's declaration
map and the count is printed. `tools/headerabi.py` independently refuses to
read a header whose `STARKBANK_API` line count does not match the number of
declarations it extracted, and `tests/header/test_headerabi.py` proves that
guard fires.

## Result on this machine

macOS 25.6 (Darwin arm64), Apple clang as `cc` and `c++`, Python 3.12, cffi in
a throwaway venv at `/tmp/starkbank-sdk-c-gate-venv` — never in the user's
environment, because cffi is a gate dependency and not a build or runtime one.

```
gate: house rules
houserules: 9 rules, no violations
gate: C89  (cc -std=c89 -pedantic-errors)
gate: C++98 (c++ -x c++ -std=c++98)
gate: cffi
cffi: 114 declarations parsed, 155 constants read
gate: tooling negative cases
tooling: all cases passed
gate: bindings regenerate byte-identically
gate: OK
```

114 declarations, 155 constants, and `make check-exports` derives the same 114
symbols from the header's `STARKBANK_API` lines with no difference in either
direction.

## Pending a Windows CI runner

Neither MSVC nor a real Delphi compiler exists on this machine, and core-c has
no Windows runner today either (`ubuntu-latest`, `macos-latest`). sdk-c is the
first in the tier to need one. Until it exists, these are untested:

1. **MSVC `/TC /W4 /permissive-`.** Expected to pass — the header is C89 and
   uses no VLAs, no complex, no designated initializers — but expected is not
   tested. This is the check that most directly falsifies design risk 2 and it
   is the one still missing.
2. **`STARKBANK_CALL` as `__cdecl` on Win32.** Spelled explicitly because a
   Delphi host defaults to `stdcall`; getting it wrong corrupts the stack on
   the first call and is invisible on every other platform, which is exactly
   the failure a local gate cannot catch.
3. **`bindings/starkbank.pas` in a real Delphi compiler.** The unit is emitted
   and reviewable but has never been compiled. Specifically unverified: the
   `&`-escaped reserved-word parameters (`&out`, `&type`), the `type Pointer`
   handle declarations, and the `{$IFDEF MSWINDOWS}` library-name block.
4. **`bindings/Starkbank.cs` under a real .NET runtime.** `DllImport` with
   `CallingConvention.Cdecl` is declared throughout, but no P/Invoke call has
   been made. `const char *` returns are deliberately `IntPtr` rather than a
   marshalled `string`, because the pointer is borrowed and the runtime would
   otherwise try to free memory it does not own — that decision is the one most
   worth exercising first.
5. **`__declspec(dllexport)` / `dllimport` under `STARKBANK_BUILD_SHARED` and
   `STARKBANK_USE_SHARED`.** Both branches are unreachable on this host.
6. **A `.def` file** derived from the header by the same grep as `exports.txt`.
   Not written yet; it belongs with the DLL, in build-plan step 5.

Build-plan step 5 owns the Windows runner. Until it lands, treat the Windows
and Delphi paths as the least-tested surface in this repo, which is what the
design already says they are.

## Deliberately not in the gate

- **`starkbank.hpp`.** The optional C++ wrapper is expanded from the X-macro
  tables, and the tables arrive with the engine in step 2.
- **A drift check against sdk-python.** `tools/drift.py` is step 4. Two
  divergences are already known and recorded below rather than being silently
  encoded in the header.
- **Compiling against a real `libstarkbank.a`.** There is not one yet. The C89
  and C++98 checks are `-fsyntax-only` for that reason, and they become link
  checks in step 2.

## Divergences from the design document, adjudicated

sdk-python is normative for the verb surface and the field set (design §1).
Two places where the design document and python disagree, and what the header
does:

1. **`transfer.Log.pdf` does not exist.** Design §7 says Transfer has "its own
   Log with pdf". `sdk-python/starkbank/transfer/log/__log.py` declares only
   `get`, `query` and `page`, and there is no `get_content` anywhere in it. The
   header follows python and declares no `starkbank_transfer_log_pdf`. The
   Transfer receipt is `starkbank_transfer_pdf`. This costs the first slice one
   of its `get_content` exercises; `starkbank_invoice_log_pdf` still covers the
   Log-with-content shape.
2. **`Transfer.accountType` is optional, not required.** python's
   `Transfer.__init__` takes `account_type` as a positional parameter with no
   default, which reads as required, but its own docstring says
   `[string, default "checking"]` and sdk-go's struct marks it
   `json:",omitempty"` under "Parameters (optional)". Two of three sources say
   optional and the third is a Python calling-convention artifact rather than a
   statement about the wire, so the field is `CREATE` without `REQUIRED`. This
   is the shape of the `type.conflict` case `tools/drift.py` must treat as a
   hard stop, and it should be seeded into `tests/reference/known-drift.json`
   with this reasoning when that file is written in step 4.

## Header comments amended by the engine (step 2)

Four claims in the step-1 header turned out to be promises the engine could
not keep honestly. They were amended rather than implemented, and each is
recorded here because the field-comment blocks are the only documentation a
consumer with no compiler ever reads.

1. **`starkbank_entity_string` no longer promises `STARKBANK_ERROR_MASKED`.**
   Masking is `checks.py:33`, which is about datetimes. Applying it to every
   STRING would turn a description containing an asterisk into an error, and
   would hide the fact that a masked tax id *is* `"***.345.678-**"` - the
   value the API sent, and the one a caller should display. `MASKED` is
   returned by `starkbank_entity_datetime` only.
2. **`starkbank_entity_free` no longer promises a safe double free.** Every
   handle carries a tag word checked on entry and cleared on free, which
   rejects a wrong or stale handle from an FFI host. It cannot make a double
   free safe: reading a word out of a block already returned to the allocator
   is undefined however it is tagged, and ASan reports it. The suite tests the
   guard it does have - a pointer that is not one of our handles - and does
   not test the one it does not.
3. **`starkbank_invoice_create` no longer promises to name the missing key.**
   The check happens locally and no request is made, which is the part that
   matters, but this ABI has no channel to carry a name back.
   `starkbankEntityRequiredMissing` computes it and is kept internal, so
   promoting it to an entry point in step 4 is one line.
4. **A transport failure surfaces as `STARKCORE_ERROR_UNKNOWN`, not
   `STARKCORE_ERROR_TRANSPORT`.** core-c hands a failed transport to the status
   mapper as python's synthetic status 0, which is neither 200 nor 400 nor
   500, so core-python raises `UnknownError`. This tier forwards the code
   unchanged rather than improving it: a code a caller cannot match against
   core-python's documented behaviour is a worse code. `ERROR_TRANSPORT` is
   what a host's own `starkbank_transport_fn` returns, not what a verb does.

## Engine decisions worth a second opinion (step 2)

- **A params bag accepts a table field only when it is patchable or the query
  list names it.** Otherwise a filter on any return-only field would be
  accepted locally and ignored by the API. `-DSTARKBANK_LOOSE_QUERY` removes
  the name check for queries only; `make test-loose` builds and runs the suite
  that way, and asserts writes stay strict in that build.
- **`limit` and `cursor` are legal on every params bag** whether or not a
  table lists them. Pagination is core's, not any resource's, and forty-three
  chances to forget `cursor` is forty-three bugs.
- **An unresolvable polymorphic shape counts as one unknown, not one per key.**
  An `Event.log` whose subscription this build predates is one gap in the
  tables, and counting its keys individually would make the number depend on
  the size of the object rather than on the size of the gap.

## Slice decisions and divergences (step 3)

The four slice resources are Invoice, Transfer, Event and Balance. Everything
below is a place where this library and sdk-python do not produce identical
bytes, or where a judgement was made that step 4's `known-drift.json` should
carry. Everything not listed here matches python byte for byte, proven by
`tests/reference/slice.json` and `make test-slice`.

### Divergences from sdk-python, deliberate

1. **`invoice.qrcode` sends no `size` when the caller passes 0.** python's
   `size` defaults to 7 and is *always* sent, so python's request carries
   `?size=7` even when the caller never mentioned it. This library sends
   nothing for 0 and lets the API apply its own default, which is the same
   result by a different route and the only way a C signature can express
   "unset" without a sentinel the caller has to know. A caller passing an
   explicit size produces python's exact request, which is what the golden
   drives. This is a `known-drift.json` entry, not a bug.
2. **An integral float is spelled as an integer.** starkcore's DOM has one
   number type and prints `10.0` as `10`; python's `json` keeps the float it
   was handed. Both are the number ten and the API accepts either. The
   recorder and the suite canonicalise numbers on both sides for this reason
   and for no other - encoding, list order and string escapes are compared
   exactly, which is why `tags=war%2Csupply` and `isDelivered=False` are real
   assertions rather than normalised away.
3. **Key order and JSON separators are not compared.** Each tier signs the
   bytes it sends, so the two need not agree on the order they build a payload
   in. python's order is `dir(entity)`, which is alphabetical by *snake_case*
   attribute name and therefore not even stable under a rename that does not
   change the wire key. Comparing it would assert a python implementation
   detail, not the wire contract.

### Engine defect the slice goldens found

**`starkbank_entity_dump` kept a JSON null; python drops it.** Every reader in
the engine already reports a null as `STARKBANK_ERROR_ABSENT`, so the dump
disagreed with the rest of the engine as well as with python, and a response
carrying `"fee": null` read as a hydration diff for a field neither side
carries. Fixed in `starkc/entity.c`; `tests/run.c` now owns the regression so
the engine suite fails on it without the slice.

### Table judgements

- **`Transfer.accountType` is `CREATE` without `REQUIRED`.** python makes it
  positional without a default, which is a calling-convention artifact: its
  own docstring says `default "checking"` and sdk-go says optional. If this is
  wrong a caller omitting it gets a 400 instead of a local
  `STARKBANK_ERROR_FIELD`, which is the cheaper failure of the two.
- **`Transfer.externalId` is `CREATE` without `REQUIRED`.** The standards want
  an idempotency key on every funds movement and the header says so loudly,
  but python does not require it and python is normative for the surface.
  Making it required here would reject a payload the API accepts.
- **`Transfer.metadata` is in the table** although app-docs' spec does not
  model it. python and sdk-go both carry it. It is the case that makes the
  permissive-read counter meaningful on a real resource: without the row,
  every Transfer the API returns would report an unknown key.
- **`Split` carries only what Invoice needs.** Its own verbs (`query`, `page`,
  `get`) and the SplitReceiver family arrive with the rest of the bank
  surface; the table is already the full field set, so nothing here changes
  when they do.
- **No `transfer.Log` pdf.** sdk-python has none and python is normative for
  the verb surface. The receipt is `starkbank_transfer_pdf`.
- **`Event.log`'s subscription map is python's, verbatim and in its order.**
  Eight of its ten tables do not exist in this build, so
  `starkbankRegistryFind` returns NULL for them and the log hydrates untagged,
  permissive and counted. That is the same path a subscription invented after
  this build ships will take, which is why the case runs on every build rather
  than waiting for the API to grow one.

### The linker-enforced opt-out, verified

`HANDWRITTEN_SOURCES` is a `$(wildcard handwritten/*.c)` and not a list. With
a list, deleting the file is "no rule to make target" - a hard failure, but
not the one the design claims. With the wildcard, `make shared` fails with

```
  "_starkbank_event_parse", referenced from:
      <initial-undefines>
  ld: symbol(s) not found
```

which names the entry point that needs a human. `make check-handwritten`
guards the other direction - a symbol defined under `handwritten/` that the
header never declared, or one defined twice - and both directions were
verified by injecting the fault.

### Transfer needed zero hand-written code

`starkbank/transfer/` is three files, all table and verb macros, no functions.
That was the step-3 falsification criterion and it held.

---

## Step 4: the drift checker, the build and the generated artifacts

### `tools/reflect` exists so no Python re-implements core-c

The drift checker needs three things: the field tables as the preprocessor
actually expanded them, the endpoint core-c derives from a resource name, and
core-c's snake-to-camel rule. A Python copy of any of the three would be a
fourth source of truth that can rot, and rot in exactly the place the checker
is supposed to be watching. So `tools/reflect` is a build-only C program that
links the library's own objects, reads `starkc/internal.h` and prints the
tables as JSON; `reflect --camel` pipes names through
`starkcore_case_snake_to_camel` itself.

This is a file the design did not name. The alternative was `gcc -E` over a
registry TU and a Python parser for the expanded initialisers, which is a
parser for C that has to keep up with the engine's struct layout. Reflect is
forty lines of printing and cannot disagree with the tables, because it *is*
the tables.

Its prerequisites include the resource **headers**, not only the `.c` files.
That was a real defect while it lasted: a field table lives in a `.h`, so a
reflect built from the `.c` files alone let `check-drift` pass against a table
that had already changed. Found by mutation-testing the checker.

### Seven checks, not six

The six the design lists, plus `resource.unjoined`, which is what seeds
`known-drift.json`: a python package with no docs resource file, or a docs
slug with no python package. It is not a table check - it names surface that
does not exist here yet - but it is measured with the same inputs, and the
design seeds the accepted set from exactly this measurement.

Joining python packages to docs slugs by **flattening the slug**
(`corporate-holder` to `corporateholder`) rather than by kebab-casing a
resource name gives five findings, three of which are the three the design
predicted (`event-attempt`, `institutions`, `public-key`). Kebab-casing the
primary resource name instead gives eight, three of them artefacts of
picking the wrong "primary" resource in a package with several.

### `type.conflict` is a hard stop that can still be resolved

The design says a `type.conflict` is a hard stop and no table change lands
until a human resolves it. Taken literally that makes `check-drift` unable to
ever be green, because `Event.isDelivered` is tagged `STRING` in the docs and
is a bool in python, today, on the first resource that has one.

The mechanism: an ordinary `known-drift` entry does **not** silence a
`type.conflict`. Only an entry carrying a `resolution` of `python`, `docs` or
`table` does - a human decision, written down, naming which input won. It goes
stale like any other entry the moment the conflict is fixed upstream. The unit
suite has a case for each half: a plain exemption fails, an adjudicated one
passes, and an unknown resolution value fails.

### The pin is advisory locally and required for a release

`tests/reference/sdk-python.sha` pins the sdk-python commit the tables and
goldens were derived from. `drift.py` reports a mismatch on stderr and carries
on; `make check-drift-pinned` refuses to compare at all. A developer with a
branch checked out must still be able to run the checker, and a tag must not
be cut against an unknown upstream. The pin names sdk-python's default branch
`master` at `575279bf`, not the feature branch checked out on this machine.

### What the checker found on the tables as they stand

Ten findings, all accepted and each with an owner and a reason: Split's three
deferred verbs, the `transactionIds` filter the docs document and python does
not accept, the five unjoined names, and the `Event.isDelivered` tag conflict
resolved in python's favour. No `field.new`, no `field.gone`, no
`field.rename`, no `endpoint.changed`: the tables and python agree exactly on
the field sets, and every endpoint this library can reach is one the docs
document.

One header comment was changed to make it machine-readable rather than
left to rot: `Invoice.Payment` said "All read-only." in prose, which no parser
should have to understand when `(ro)` is the marker every other block uses.

### Mutation-testing the checker

Five faults injected, each caught, each reverted:

1. a row deleted from the Invoice table - `field.new` and `comment.stale`;
2. a `DELETE_ID` verb Balance does not have - `verb.gone` and
   `endpoint.changed`;
3. a query key the docs do not list - `query.gone` and `comment.stale`;
4. `Invoice.due` retyped `DATETIME` - the `type.conflict` hard stop;
5. `brcode` renamed `brCode` - `field.rename`, which is the finding no fixture
   can produce, since the goldens are recorded from python and go stale in
   lockstep with the tables.

### The samples are programs, and the emitter cannot write into the library

30 samples, one per mechanical verb, each a complete program that compiles
under the library's own warning set with `-Werror`. The boilerplate is bracketed
with `/*<*/` and `/*>*/` so the published snippet is the size of the python
one. They use wire-key string literals rather than the `STARKBANK_INVOICE_*`
constants: a reader arriving from the python docs matches the sample to the
field list at a glance, and the constants are one line above in the header for
anyone who wants the compiler's help. Worth revisiting when the docs tab ships.

`tools/emit.py` refuses to write anywhere but `bindings/`, `samples/` and
`include/starkbank_fields.h`, and the predicate is tested against real
in-repo paths. It is tested as a **predicate**: calling `writeOrCheck` on
`starkc/entity.c` to prove it refuses is how a red test destroys a source
file, because the assertion only runs after the call. That is not
hypothetical - it happened here, during the red phase of that very test, and
the file was recovered and then verified by rebuilding it and comparing the
object byte for byte with the one built before the accident.

### Build

- `make bundle` merges the objects with core-c, ecdsa-c and libsecp256k1 into
  `libstarkbank_full.a` and links `libstarkbank_full.dylib` / `.so`, exporting
  only `starkbank_*`. `check-exports` checks the bundle too when it is built:
  a leaked `starkcore_` symbol can rebind a host's own copy at load time,
  which is a security bug and not a packaging one.
- `make bundle-curl` adds core-c's optional libcurl transport, as separate
  artifacts (`libstarkbank_full_curl.*`) rather than a flag on the same ones.
  A target-specific variable cannot change what an already up-to-date file
  contains, and a bundle that silently has no transport inside it is worse
  than one that does not exist - which is what the first cut here shipped
  until `nm` said otherwise. Separate also because linking libcurl into a POS
  terminal is a decision.
- `make tsan` shares one client across eight threads. Scope, honestly:
  libstarkcore is a prebuilt archive here, so a race inside core-c's key cache
  is core-c's own `make tsan` to catch. What this proves is that this tier
  adds no shared mutable state of its own.
- `make install` and `starkbank.pc` with `Requires: starkcore starkecdsa`.
  `make print-ldflags` prints the static link order so nobody guesses it.

### Windows

The CI job compiles only, and it is documented as red until the sibling
repositories are reachable - as every other job in that workflow is. It builds
the public header as its own MSVC translation unit and every source under
`/TC /W4 /permissive-`. It cannot link: core-c has no Windows build. This is
committed now so the Windows evidence starts accumulating the day the siblings
are published, rather than at release; it is still not the week-one experiment
design risk 2 asks for, which needs a Delphi translator and a real P/Invoke
round trip.
