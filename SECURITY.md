# Security policy

## Reporting a vulnerability

Report privately through GitHub: **Security → Report a vulnerability** on this
repository. Please do not open a public issue for anything that could affect a
key, a signature, the acceptance of a webhook, or the bytes of a payment
request. Every report is acknowledged, and a confirmed issue is fixed in the
next release with credit to the reporter unless they prefer none.

## Scope

In scope, because this library decides them:

- **Memory safety across the ABI.** Every handle carries a magic word checked
  on entry, every accessor hands back a borrowed pointer with a documented
  lifetime, and three entry points transfer ownership. A wild pointer, a
  use-after-free or a double free reachable from a documented call sequence is
  a vulnerability here, not a caller error.
- **Untrusted input.** `starkbank_parse_and_verify` runs on a webhook body
  before any signature has been accepted, and hydration runs on whatever the
  transport returned. Both are fuzz targets.
- **Strict writes.** A payload key that is silently dropped or silently
  retyped moves money wrong. A write the table should have refused, or a write
  that reaches the wire under a different key, is in scope.
- **The bundle's export list.** `make bundle` embeds core-c, ecdsa-c and
  libsecp256k1 and must export only `starkbank_*`. A leaked `starkcore_` or
  `cJSON_` symbol can rebind a host's own copy at load time, which is a
  security bug and not a packaging one.

Out of scope here, in scope in the repository that owns it:

- Request signing, casing, envelopes, pagination and status mapping -
  `starkinfra/core-c`.
- Curve arithmetic, key parsing and DER handling - `starkbank/ecdsa-c` and
  `libsecp256k1`.
- The API itself.

## What this library never does

- It has no HTTP client and no TLS. The host supplies a transport, so this
  library holds no CA bundle and makes no trust decisions of its own.
- It has no global mutable state and no default user. A client is an argument.
- It never logs. Nothing here can print a key, an access id or a payload.
