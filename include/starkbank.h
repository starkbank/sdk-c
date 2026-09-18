/*
 * starkbank - the Stark Bank resource tier in C, over starkcore.
 *
 * The behavioural reference is sdk-python's starkbank package: the verb
 * surface, the field set and the input coercions come from there. Nothing on
 * the wire is decided here. Casing, endpoint derivation, envelope keys,
 * pagination arithmetic and status mapping all belong to starkcore and are
 * called at run time, never reimplemented.
 *
 * This header is the ABI, and follows starkcore.h's restrictions exactly:
 *
 *   - C89 constructs only: no // comments, no inline, no variadic macros,
 *     no bool, no stdint, no designated initializers
 *   - no long anywhere: long is 32 bits on Win64 and 64 bits on LP64
 *   - every object is an opaque pointer, so layout never crosses the boundary
 *   - every buffer this library allocates is released with starkbank_free,
 *     never the caller's free: on Windows the two may be different runtimes
 *   - no HTTP and no TLS. The host supplies a transport, or links the
 *     optional curl transport
 *   - no global mutable state. sdk-python's module-level starkbank.user has no
 *     equivalent here: the client is the first argument of every verb
 *   - no entry point aborts, and every out parameter is set to NULL or left
 *     untouched on failure, so a host that ignores a return code still has a
 *     pointer it can safely free
 *
 * Fields are not part of the ABI
 * ------------------------------
 * There is no per-field entry point and no per-resource struct. An entity is
 * one API object plus a tag naming its resource, and it is read and written
 * through the generic accessors below, keyed by wire field name and validated
 * against that resource's field table. Adding a field upstream therefore adds
 * no exported symbol and breaks no host: a build of this library that predates
 * the field still returns it through starkbank_entity_json, and a build that
 * postdates it needs no recompilation of the caller.
 *
 * The cost is that starkbank_entity_string(invoice, "brcde", &s) is a run-time
 * STARKBANK_ERROR_FIELD rather than a compile error. The per-resource field
 * name constants below (STARKBANK_INVOICE_BRCODE and friends) give the
 * compiler the typo back for callers who want it.
 *
 * Ownership - five rules, no exceptions
 * -------------------------------------
 *   1. Accessors never allocate. Every const char * handed back is borrowed
 *      from the entity's own document and stays valid until that entity is
 *      freed or mutated. There is nothing to free.
 *   2. An entity from _new, _get, _update, _payment or _clone is yours:
 *      release it with starkbank_entity_free.
 *   3. Entities inside a starkbank_list are borrowed and are freed by
 *      starkbank_list_free. starkbank_list_append takes ownership of what you
 *      hand it. To keep one past the list, starkbank_entity_clone.
 *   4. starkbank_iter_next yields an entity borrowed until the next call -
 *      deliberately the same rule as starkcore_stream_next, so one rule covers
 *      both tiers. Clone to keep.
 *   5. Blobs from _pdf, _qrcode and _dump, and cursor strings, are released
 *      with starkbank_free. Plain free() is never correct: in the bundled
 *      build this library and the host may hold different runtimes.
 *
 * Threading
 * ---------
 * Inherited from starkcore, since the handles wrap its handles. A
 * starkbank_client is immutable once configured and may be shared across
 * threads, including concurrent starkbank_parse_and_verify calls. An entity,
 * list or iterator is single-owner: one thread builds and frees each. Configure
 * a client fully before publishing it to other threads; the setters are not
 * locked. A host transport must be reentrant.
 */

#ifndef STARKBANK_H
#define STARKBANK_H

#include <stddef.h>
#include <starkcore.h>

#if defined(_WIN32)
#  if defined(STARKBANK_BUILD_SHARED)
#    define STARKBANK_API __declspec(dllexport)
#  elif defined(STARKBANK_USE_SHARED)
#    define STARKBANK_API __declspec(dllimport)
#  else
#    define STARKBANK_API
#  endif
#else
#  if defined(STARKBANK_BUILD_SHARED)
#    define STARKBANK_API __attribute__((visibility("default")))
#  else
#    define STARKBANK_API
#  endif
#endif

/* Spelled explicitly on Win32, where a Delphi host defaults to stdcall and
   would otherwise corrupt the stack on the first call it makes. */
#if defined(_WIN32) && !defined(_WIN64)
#  define STARKBANK_CALL __cdecl
#else
#  define STARKBANK_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ return codes */
/*
 * Every starkcore code is returned unchanged, so a caller compares against
 * STARKCORE_OK and switches on STARKCORE_ERROR_* exactly as it would one tier
 * down. The codes this tier adds all sit below -100, which is why no value
 * here can ever collide with a code starkcore adds later.
 * starkbank_strerror forwards anything it does not recognise to
 * starkcore_strerror, so one call describes both tiers.
 */
#define STARKBANK_OK                 0      /* the same value as STARKCORE_OK */
#define STARKBANK_ERROR_FIELD    (-101)     /* no such field on this resource, or not writable */
#define STARKBANK_ERROR_TYPE     (-102)     /* field exists, wrong accessor for its type */
#define STARKBANK_ERROR_ABSENT   (-103)     /* field is valid, the object does not carry it */
#define STARKBANK_ERROR_MASKED   (-104)     /* server-redacted value; checks.py:33 */
#define STARKBANK_ERROR_RESOURCE (-105)     /* entity belongs to another resource */
#define STARKBANK_ERROR_ABI      (-106)     /* the linked starkcore's ABI is not the one this was built against */

/*
 * Bumped only when the meaning of a handle, a code or a prototype changes.
 * Never when a field or a resource is added: that is the point of keeping
 * fields out of the ABI, and a host that bumps for a new field has lost it.
 */
#define STARKBANK_ABI_VERSION 1

/* Release of the library. The Makefile parses this line for the archive
   version and the soname, so it is the single source. */
#define STARKBANK_VERSION "0.1.0"

/* ------------------------------------------------------------ enumerations */
/* Re-declared rather than aliased so a host binds one header and one library.
   The values are starkcore's and the two are checked equal at build time. */

#define STARKBANK_ENVIRONMENT_PRODUCTION 0
#define STARKBANK_ENVIRONMENT_SANDBOX    1

#define STARKBANK_LANGUAGE_EN_US 0
#define STARKBANK_LANGUAGE_PT_BR 1

#define STARKBANK_METHOD_GET    0
#define STARKBANK_METHOD_POST   1
#define STARKBANK_METHOD_PUT    2
#define STARKBANK_METHOD_PATCH  3
#define STARKBANK_METHOD_DELETE 4

/* ------------------------------------------------------------- field types */
/*
 * The type a field's table row declares. It selects the accessor that reads
 * the field and the wire form a writer produces, and it is what
 * starkbank_resource_field_type_at reflects so a binding generator can emit a
 * typed wrapper without reading this header.
 */
#define STARKBANK_FIELD_STRING            0
#define STARKBANK_FIELD_AMOUNT            1   /* integer cents; see the note on doubles below */
#define STARKBANK_FIELD_RATE              2   /* a percentage, fractional. ex: 2.5 */
#define STARKBANK_FIELD_SECONDS           3   /* python timedelta, sent as integer seconds */
#define STARKBANK_FIELD_NUMBER            4
#define STARKBANK_FIELD_BOOL              5
#define STARKBANK_FIELD_DATE              6
#define STARKBANK_FIELD_DATETIME          7
#define STARKBANK_FIELD_DATE_OR_DATETIME  8   /* Invoice.due: a date means a scheduled invoice */
#define STARKBANK_FIELD_LIST_STRING       9
#define STARKBANK_FIELD_LIST_OBJECT      10   /* free-form maps: discounts, descriptions */
#define STARKBANK_FIELD_LIST_RESOURCE    11   /* rules, splits: entities of a named sub-resource */
#define STARKBANK_FIELD_RESOURCE         12   /* invoice.Log.invoice, Invoice.Payment */
#define STARKBANK_FIELD_OBJECT           13   /* Transfer.metadata: an opaque map */

/* ------------------------------------------------------------ field flags */
/* A field carrying neither CREATE nor PATCH is return-only and no writer will
   accept it. Writes are strict on purpose: a payload key dropped in silence
   moves money wrong, and catching it here is the field table's main reason to
   exist. The one documented way around it is starkbank_entity_set_json_raw. */
#define STARKBANK_FLAG_REQUIRED  1   /* required on create */
#define STARKBANK_FLAG_CREATE    2   /* accepted in a create payload */
#define STARKBANK_FLAG_PATCH     4   /* accepted in an update payload */

/* --------------------------------------------------------- opaque handles */

typedef struct starkbank_user starkbank_user;      /* a project or organization credential */
typedef struct starkbank_client starkbank_client;  /* owns the user; pins host and sdk version */
typedef struct starkbank_entity starkbank_entity;  /* one API object, tagged with its resource */
typedef struct starkbank_list starkbank_list;      /* an owned array of entities: a page, a batch */
typedef struct starkbank_iter starkbank_iter;      /* a cursor-paged generator over starkcore_stream */

/*
 * The same object starkcore returns, under this library's name, so a host
 * binds one header. One lifetime rule, one free.
 */
typedef starkcore_errors starkbank_errors;

/*
 * The transport seam, and the only place starkcore's own handles are named in
 * a signature. A host that installs its own HTTP stack needs to build the
 * response it received, so the two builders it needs are re-exported below.
 */
typedef starkcore_headers starkbank_headers;
typedef starkcore_response starkbank_response;

typedef int (STARKBANK_CALL *starkbank_transport_fn)(
    void *context,
    int method,
    const char *url,
    const starkbank_headers *headers,
    const char *body,
    size_t body_len,
    int timeout_seconds,
    starkbank_response **out);
/* Perform exactly one request, build the reply with starkbank_response_new and
   return STARKBANK_OK. Return STARKCORE_ERROR_TRANSPORT for any network, DNS,
   TLS or timeout failure. Write body_len bytes of body verbatim: those exact
   bytes were signed. Must be reentrant if the host calls from several threads. */

/* ----------------------------------------------------------------- library */

STARKBANK_API int STARKBANK_CALL starkbank_abi_version(void);
/* Refuse to run if this differs from the STARKBANK_ABI_VERSION you built against. */

STARKBANK_API const char * STARKBANK_CALL starkbank_version(void);
/* Always STARKBANK_VERSION. This is also the sdk_version this library reports
   to starkcore, so it is the token the User-Agent carries. Never freed. */

STARKBANK_API const char * STARKBANK_CALL starkbank_core_version(void);
/* The starkcore actually linked in, which in a bundled build a host cannot ask
   for any other way. Never freed. */

STARKBANK_API const char * STARKBANK_CALL starkbank_strerror(int code);
/* Static English description of any code from either tier; never NULL, never
   freed. Codes it does not own are forwarded to starkcore_strerror. */

STARKBANK_API void STARKBANK_CALL starkbank_free(void *pointer);
/* Releases any buffer or string this library returned through an out
   parameter: _pdf and _qrcode blobs, _dump text, page cursors. Forwards to
   starkcore_free. A NULL pointer is accepted and ignored. Calling the C
   library's free() on one of these is never correct. */

/* -------------------------------------------------------------------- user */
/*
 * A user is a credential. Unlike starkcore, where the user is borrowed and
 * must outlive the client, here the client TAKES OWNERSHIP of the user: that
 * ordering is the lifetime bug an FFI host reliably writes, and this tier
 * absorbs it rather than forwarding it. Free a user yourself only if you never
 * handed it to a client.
 */

STARKBANK_API int STARKBANK_CALL starkbank_project_new(const char *id, int environment,
    const char *private_key_pem, starkbank_user **out);
/* Project credential; the access id is "project/{id}". The PEM is checked here,
   so a bad key fails at construction and not at the first request. */

STARKBANK_API int STARKBANK_CALL starkbank_organization_new(const char *id, int environment,
    const char *private_key_pem, const char *workspace_id, starkbank_user **out);
/* Organization credential; NULL or "" for workspace_id omits the
   "/workspace/{id}" segment, as python's Organization does. */

STARKBANK_API int STARKBANK_CALL starkbank_organization_replace(const starkbank_user *organization,
    const char *workspace_id, starkbank_user **out);
/* A new Organization with the same id, environment and key on another
   workspace; the original is untouched. python's organization.replace. */

STARKBANK_API const char * STARKBANK_CALL starkbank_user_access_id(const starkbank_user *user);
/* The exact Access-Id header value, for a host that signs its own requests.
   Borrowed, valid while the user lives. NULL for a user that carries no id. */

STARKBANK_API int STARKBANK_CALL starkbank_user_environment(const starkbank_user *user);
/* STARKBANK_ENVIRONMENT_* of this credential; it selects the base URL. */

STARKBANK_API void STARKBANK_CALL starkbank_user_free(starkbank_user *user);
/* Zeroes the private key material before releasing it. Do NOT call this on a
   user a client has taken: starkbank_client_free owns it from that point. */

/* ------------------------------------------------------------------ client */

STARKBANK_API int STARKBANK_CALL starkbank_client_new(starkbank_user *user, starkbank_client **out);
/* TAKES OWNERSHIP of user, on failure as well as on success, so one error path
   serves both and a host cannot leak the credential by ignoring a code.
   Returns STARKBANK_ERROR_ABI when starkcore_abi_version() is not the
   STARKCORE_ABI_VERSION this library was compiled against - the check the
   frozen headers ask every host to make, made once, here, for free.
   Defaults follow starkcore: api version "v2", en-US, 15s, fractional
   Access-Time, and no transport until one is installed. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_transport(starkbank_client *client,
    starkbank_transport_fn transport, void *context);
/* Installs the host's HTTP hook. Without a transport only signing works.
   A POS terminal keeps its own stack and arrives through here. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_curl_transport(starkbank_client *client);
/* Installs the optional libcurl transport. Returns STARKCORE_ERROR_NO_TRANSPORT
   in a build made without it, so a host can try this and fall back rather than
   failing to link. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_language(starkbank_client *client, int language);
/* STARKBANK_LANGUAGE_*; it is the language API error messages come back in. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_timeout(starkbank_client *client, int seconds);
/* Per-request timeout handed to the transport; 15 by default. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_user_agent_prefix(starkbank_client *client,
    const char *prefix);
/* Leading User-Agent token for a host that resells this library; NULL or "" for none. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_max_response_size(starkbank_client *client,
    size_t bytes);
/* Rejects larger bodies with STARKCORE_ERROR_SIZE; 0, the default, is no cap. */

STARKBANK_API int STARKBANK_CALL starkbank_client_set_user(starkbank_client *client, starkbank_user *user);
/* Swaps the credential for a workspace hop. TAKES OWNERSHIP of the new user and
   frees the old one. Not locked: do this before other threads see the client. */

STARKBANK_API void STARKBANK_CALL starkbank_client_cache_clear(starkbank_client *client);
/* Drops the Stark public key this client has cached for signature verification. */

STARKBANK_API void STARKBANK_CALL starkbank_client_free(starkbank_client *client);
/* Releases the client AND the user it owns. A NULL client is accepted. */

/* ------------------------------------------------------------- transport seam */
/* Exactly enough of starkcore's response and header API for a host to write a
   starkbank_transport_fn without binding a second library. */

STARKBANK_API int STARKBANK_CALL starkbank_response_new(int status, const unsigned char *content,
    size_t content_len, const starkbank_headers *headers, starkbank_response **out);
/* What a transport builds from what it received; headers may be NULL. Copies
   content. Ownership passes to this library when the transport returns OK. */

STARKBANK_API int STARKBANK_CALL starkbank_headers_count(const starkbank_headers *headers);
STARKBANK_API const char * STARKBANK_CALL starkbank_headers_name_at(const starkbank_headers *headers, int index);
STARKBANK_API const char * STARKBANK_CALL starkbank_headers_value_at(const starkbank_headers *headers, int index);
/* Iterate the request headers this library built, so the transport can copy
   them onto its own request object. All borrowed; NULL when out of range. */

/* ------------------------------------------------------------------ errors */
/* A 400 from the API carries a list of coded errors. Every verb takes an
   errors out parameter, and every one of them may be NULL if you do not want
   them. When it is not NULL it is set to NULL on success and on any failure
   that is not an API error, so the free below is always safe to call. */

STARKBANK_API int STARKBANK_CALL starkbank_errors_count(const starkbank_errors *errors);
STARKBANK_API const char * STARKBANK_CALL starkbank_errors_code_at(const starkbank_errors *errors, int index);
/* ex: "invalidAmount". Borrowed; NULL when out of range. */
STARKBANK_API const char * STARKBANK_CALL starkbank_errors_message_at(const starkbank_errors *errors, int index);
/* Human-readable, in the client's language. Borrowed; NULL when out of range. */
STARKBANK_API void STARKBANK_CALL starkbank_errors_free(starkbank_errors *errors);
/* Errors are the caller's to free. A NULL list is accepted. */

/* ------------------------------------------------------------------ entity */
/*
 * One object from the API, or one you are building to send. It carries the
 * document it came from plus a tag naming its resource, and every accessor
 * below is validated against that resource's field table.
 *
 * Reads are permissive and writes are strict. A read of a key the document
 * carries and the table does not returns the value and bumps
 * starkbank_entity_unknown_count, so a field the API added surfaces as a CI
 * failure in this repo rather than as an error in a caller's production path.
 * A write to a key the table does not carry, or carries without the right
 * flag, is STARKBANK_ERROR_FIELD.
 *
 * Absent is not zero. STARKBANK_ERROR_ABSENT is returned for a missing key and
 * for JSON null, and *out is left untouched. This is not pedantry: an Invoice
 * amount of 0 is legal and means "accept whatever the payer sends", so 0 and
 * absent must be distinguishable and no sentinel would be safe.
 *
 * Amounts are integer cents carried in a double. int overflows at
 * R$ 21,474,836.47, which is inside normal ledger range; long is banned from
 * this ABI because it is 32 bits on Win64; long long is not C89. A double is
 * exact for every integer below 2^53, which is R$ 90 trillion in cents, and it
 * is already what starkcore_json_number hands over, so nothing is gained or
 * lost in precision at this tier. If exactness beyond 2^53 is ever needed the
 * answer is a decimal-string accessor, not a wider integer.
 */

STARKBANK_API const char * STARKBANK_CALL starkbank_entity_resource(const starkbank_entity *entity);
/* The resource this entity is tagged with, ex: "Invoice", "InvoiceLog",
   "Invoice.Rule". Borrowed and static; never freed. NULL for a NULL entity. */

STARKBANK_API const char * STARKBANK_CALL starkbank_entity_id(const starkbank_entity *entity);
/* Shorthand for the "id" field, which almost every resource has. Borrowed.
   NULL when the object carries no id, which is normal before a create. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_has(const starkbank_entity *entity, const char *field);
/* Nonzero when the document carries a non-null value for field. The cheap
   pre-check that keeps STARKBANK_ERROR_ABSENT out of a hot loop. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_string(const starkbank_entity *entity,
    const char *field, const char **out);
/* STRING fields, and the vocabulary constants below are the values to compare
   against. Enums stay strings deliberately: the docs' list for invoice status
   omits "registered", which python uses, so an int enum would have no way to
   express a status the API returns every day. Borrowed; *out is untouched on
   failure.
   A redacted value comes back verbatim rather than as STARKBANK_ERROR_MASKED:
   a masked tax id IS "***.345.678-**" and that is what the API sent, while a
   description containing an asterisk is nobody's redaction. Masking is
   reported only where python checks for it, on the datetime reader below. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_amount(const starkbank_entity *entity,
    const char *field, double *out);
/* AMOUNT fields, in integer cents. STARKBANK_ERROR_TYPE on any other type. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_number(const starkbank_entity *entity,
    const char *field, double *out);
/* NUMBER, RATE and SECONDS fields. A RATE is a percentage: 2.5 means 2.5%. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_bool(const starkbank_entity *entity,
    const char *field, int *out);
/* BOOL fields; *out is 0 or 1. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_datetime(const starkbank_entity *entity,
    const char *field, int *year, int *month, int *day,
    int *hour, int *minute, int *second, int *out_has_time);
/* One reader for DATE, DATETIME and DATE_OR_DATETIME, delegating to
   starkcore_datetime_parse. *out_has_time is 0 for a plain date, and on a
   DATE_OR_DATETIME field that is the whole distinction: a date in Invoice.due
   produces a scheduled invoice whose discounts the payer is shown. Every out
   pointer may be NULL if you do not want that component.
   STARKBANK_ERROR_MASKED when the server redacted the value, mirroring
   checks.py:33 - a redaction, not a parse failure. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_list_size(const starkbank_entity *entity,
    const char *field, int *out);
/* Element count of any LIST_* field; 0 rather than ABSENT for an empty list. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_list_string_at(const starkbank_entity *entity,
    const char *field, int index, const char **out);
/* One element of a LIST_STRING field. Borrowed. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_list_entity_at(const starkbank_entity *entity,
    const char *field, int index, const starkbank_entity **out);
/* One element of a LIST_RESOURCE or LIST_OBJECT field. The element is borrowed
   and carries its own tag, so reading it is validated against the sub-
   resource's table: starkbank_entity_string(rule, "key", &k) checks against
   Invoice.Rule, not against Invoice. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_entity(const starkbank_entity *entity,
    const char *field, const starkbank_entity **out);
/* A RESOURCE or OBJECT field: invoice.Log.invoice, Transfer.metadata.
   Borrowed, tagged, and read with these same accessors - which is why
   Event.log, whose resource varies with the subscription, needs no special
   case in a caller. */

STARKBANK_API const starkcore_json * STARKBANK_CALL starkbank_entity_json(const starkbank_entity *entity);
/* The underlying document, for a C or C++ caller that wants starkcore
   directly. The one starkcore type in this header's signatures, and the escape
   hatch for anything the tables do not model. Borrowed; never freed here. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_dump(const starkbank_entity *entity,
    char **out, size_t *out_len);
/* Table-driven serialization with sorted keys, so two runs and two machines
   produce the same bytes. This is what the hydration goldens compare against
   python's json.dumps(api.api_json(obj), sort_keys=True). Free with
   starkbank_free; *out is NULL on failure. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_unknown_count(const starkbank_entity *entity);
/* How many keys the document carries that this build's table does not know,
   counted recursively. The offline goldens and the nightly sandbox job both
   assert this is 0, which is how a field added upstream becomes a red build
   here instead of a silent gap. -1 for a NULL entity. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_string(starkbank_entity *entity,
    const char *field, const char *value);
/* A NULL value writes JSON null, which the outbound cast then drops - the same
   as leaving a python keyword argument at None. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_amount(starkbank_entity *entity,
    const char *field, double cents);
/* Integer cents. A fractional value is STARKCORE_ERROR_ARGUMENT: rounding
   somebody's money silently is worse than refusing it. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_number(starkbank_entity *entity,
    const char *field, double value);
/* NUMBER and RATE fields. Printed locale-independently by starkcore. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_bool(starkbank_entity *entity,
    const char *field, int value);

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_date(starkbank_entity *entity,
    const char *field, int year, int month, int day);
/* "%Y-%m-%d" through starkcore_date_format, never strftime, which is
   locale-dependent. STARKBANK_ERROR_TYPE on a DATETIME field: on a
   DATE_OR_DATETIME field the choice of writer is the semantic. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_datetime(starkbank_entity *entity,
    const char *field, int year, int month, int day, int hour, int minute, int second);
/* "%Y-%m-%dT%H:%M:%S+00:00", UTC, through starkcore_datetime_format.
   STARKBANK_ERROR_TYPE on a DATE field. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_seconds(starkbank_entity *entity,
    const char *field, int seconds);
/* A SECONDS field: python's timedelta, on the wire as a whole number of
   seconds. Invoice.expiration is the one in the first slice. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_append_string(starkbank_entity *entity,
    const char *field, const char *value);
/* Appends to a LIST_STRING field, creating it when absent. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_append_entity(starkbank_entity *entity,
    const char *field, starkbank_entity *value);
/* Appends to a LIST_RESOURCE or LIST_OBJECT field, creating it when absent.
   TAKES OWNERSHIP of value on failure as well as on success, so one error path
   serves both. For a LIST_RESOURCE the value's tag must match the one the
   table names, or it is STARKBANK_ERROR_RESOURCE. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_json_raw(starkbank_entity *entity,
    const char *field, const char *json_text);
/* UNVALIDATED. Parses json_text and writes it under field with no table check
   at all, so a caller can send a key the API grew after this build shipped.
   The field name is not checked, the type is not checked, and nothing here
   will tell you it was wrong. Everything else in this header exists so you do
   not need this. */

STARKBANK_API int STARKBANK_CALL starkbank_entity_clone(const starkbank_entity *entity,
    starkbank_entity **out);
/* A deep copy with the same tag, which is how a borrowed entity from a list or
   an iterator outlives the thing it came from. Yours to free. */

STARKBANK_API void STARKBANK_CALL starkbank_entity_free(starkbank_entity *entity);
/* Only for an entity you own - see ownership rules 2, 3 and 4 above. NULL is
   accepted, and so is any pointer that is not an entity this library built:
   each handle carries a tag word this checks before touching anything, which
   is the guard an FFI host needs because it has no type system to lean on.
   It is a guard against a WRONG or STALE handle and not a licence to free
   twice - reading a block already returned to the allocator is undefined
   whatever is written in it, and ASan reports it as such. */

STARKBANK_API int STARKBANK_CALL starkbank_object_new(starkbank_entity **out);
/* An untagged, permissive entity: no table, no strict writes. This is how the
   free-form maps are built - an Invoice discount is {"percentage": 10.0,
   "due": "2026-10-28"} and no table can usefully describe it. Append it with
   starkbank_entity_append_entity, which takes ownership. */

/* -------------------------------------------------------------------- list */
/* An owned array of entities: a create batch on the way out, a page on the way
   back. The list owns what it holds. */

STARKBANK_API int STARKBANK_CALL starkbank_list_new(starkbank_list **out);
STARKBANK_API int STARKBANK_CALL starkbank_list_append(starkbank_list *list, starkbank_entity *entity);
/* TAKES OWNERSHIP of entity, on failure as well as on success. Do not free it
   afterwards and do not append the same entity twice. */
STARKBANK_API int STARKBANK_CALL starkbank_list_count(const starkbank_list *list);
/* -1 for a NULL list, so a caller cannot mistake an error for an empty page. */
STARKBANK_API const starkbank_entity * STARKBANK_CALL starkbank_list_at(const starkbank_list *list, int index);
/* Borrowed, valid while the list lives. NULL when out of range. */
STARKBANK_API void STARKBANK_CALL starkbank_list_free(starkbank_list *list);
/* Frees the list and every entity in it. NULL is accepted. */

/* -------------------------------------------------------------------- iter */
/* python's generator: the cursor-paged walk over a listing endpoint. Network
   failures surface from _next, not from the _query that built it, exactly as
   in starkcore_stream. */

STARKBANK_API int STARKBANK_CALL starkbank_iter_next(starkbank_iter *iter,
    const starkbank_entity **out, starkbank_errors **errors);
/* Sets *out to the next entity, or to NULL with a return of STARKBANK_OK when
   the walk is done - so the loop condition is a status check AND a NULL check.
   The entity is borrowed and valid only until the next call: clone to keep. */

STARKBANK_API const char * STARKBANK_CALL starkbank_iter_cursor(const starkbank_iter *iter);
/* The cursor that would fetch the next page, so a long walk can be
   checkpointed and resumed. Borrowed; NULL when there is no next page. */

STARKBANK_API void STARKBANK_CALL starkbank_iter_free(starkbank_iter *iter);
/* Releases the iterator and the page it is holding. NULL is accepted. */

/* ---------------------------------------------------------------- registry */
/*
 * The field tables, reflected through the ABI. This is what lets a host
 * regenerate its own typed wrappers on its own schedule instead of waiting for
 * a release of ours, and it is what tools/emit.py reads to produce the Delphi
 * unit and the C# class shipped in bindings/.
 */

STARKBANK_API int STARKBANK_CALL starkbank_resource_count(void);
STARKBANK_API const char * STARKBANK_CALL starkbank_resource_name_at(int index);
/* ex: "Invoice", "InvoiceLog", "Invoice.Rule". Borrowed and static. */
STARKBANK_API int STARKBANK_CALL starkbank_resource_field_count(const char *resource);
/* -1 for an unknown resource. */
STARKBANK_API const char * STARKBANK_CALL starkbank_resource_field_name_at(const char *resource, int index);
/* The wire key, ex: "taxId". Borrowed and static. */
STARKBANK_API int STARKBANK_CALL starkbank_resource_field_type_at(const char *resource, int index);
/* A STARKBANK_FIELD_* value; -1 when out of range. */
STARKBANK_API int STARKBANK_CALL starkbank_resource_field_flags_at(const char *resource, int index);
/* An OR of STARKBANK_FLAG_*; -1 when out of range. 0 means return-only. */

/* ------------------------------------------------------------------- parse */

STARKBANK_API int STARKBANK_CALL starkbank_parse_and_verify(const starkbank_client *client,
    const char *content, size_t content_len, const char *signature_base64,
    starkbank_entity **out, starkbank_errors **errors);
/* Verifies a webhook body against Stark's public key, then hydrates it as an
   Event - the "event" envelope key is supplied for you. STARKCORE_ERROR_SIGNATURE
   when it does not check out. Writes the client's public key cache under the
   client's lock, so concurrent calls on one client are safe. Webhook bodies are
   untrusted input and this entry point is fuzzed. */

/* =========================================================================
 *                                 Invoice
 * =========================================================================
 *
 * Fields (wire keys; * = required on create, + = also accepted in an update).
 * Checked against the field table by tools/drift.py, so this block cannot rot
 * in silence - a comment is the only documentation a consumer with no compiler
 * ever reads.
 *
 *   amount*+ AMOUNT            taxId* STRING          name* STRING
 *   due+ DATE_OR_DATETIME      expiration+ SECONDS    fine RATE
 *   interest RATE              status+ STRING         tags LIST_STRING
 *   discounts LIST_OBJECT      descriptions LIST_OBJECT
 *   rules LIST_RESOURCE("Invoice.Rule")               splits LIST_RESOURCE("Split")
 *   id pdf link brcode STRING (ro)
 *   nominalAmount fineAmount interestAmount discountAmount fee AMOUNT (ro)
 *   transactionIds LIST_STRING (ro)                   created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, status, tags, ids.
 *
 * amount = 0 is legal and means the Invoice accepts any amount the payer
 * sends. Write a date rather than a datetime into due to produce a scheduled
 * Invoice, whose discounts and interest the payer's banking interface shows.
 */

#define STARKBANK_INVOICE_AMOUNT           "amount"
#define STARKBANK_INVOICE_TAX_ID           "taxId"
#define STARKBANK_INVOICE_NAME             "name"
#define STARKBANK_INVOICE_DUE              "due"
#define STARKBANK_INVOICE_EXPIRATION       "expiration"
#define STARKBANK_INVOICE_FINE             "fine"
#define STARKBANK_INVOICE_INTEREST         "interest"
#define STARKBANK_INVOICE_DISCOUNTS        "discounts"
#define STARKBANK_INVOICE_DESCRIPTIONS     "descriptions"
#define STARKBANK_INVOICE_RULES            "rules"
#define STARKBANK_INVOICE_SPLITS           "splits"
#define STARKBANK_INVOICE_TAGS             "tags"
#define STARKBANK_INVOICE_STATUS           "status"
#define STARKBANK_INVOICE_PDF              "pdf"
#define STARKBANK_INVOICE_LINK             "link"
#define STARKBANK_INVOICE_BRCODE           "brcode"
#define STARKBANK_INVOICE_NOMINAL_AMOUNT   "nominalAmount"
#define STARKBANK_INVOICE_FINE_AMOUNT      "fineAmount"
#define STARKBANK_INVOICE_INTEREST_AMOUNT  "interestAmount"
#define STARKBANK_INVOICE_DISCOUNT_AMOUNT  "discountAmount"
#define STARKBANK_INVOICE_FEE              "fee"
#define STARKBANK_INVOICE_TRANSACTION_IDS  "transactionIds"
#define STARKBANK_INVOICE_ID               "id"
#define STARKBANK_INVOICE_CREATED          "created"
#define STARKBANK_INVOICE_UPDATED          "updated"

/* Statuses. Strings, not an enum: "registered" is absent from the docs' own
   list and the API returns it constantly, and an unknown string is merely
   unrecognised where an unknown int in an old binary is undefined. */
#define STARKBANK_INVOICE_STATUS_CREATED    "created"
#define STARKBANK_INVOICE_STATUS_REGISTERED "registered"
#define STARKBANK_INVOICE_STATUS_PAID       "paid"
#define STARKBANK_INVOICE_STATUS_CANCELED   "canceled"
#define STARKBANK_INVOICE_STATUS_EXPIRED    "expired"
#define STARKBANK_INVOICE_STATUS_OVERDUE    "overdue"
#define STARKBANK_INVOICE_STATUS_REVERSED   "reversed"

STARKBANK_API int STARKBANK_CALL starkbank_invoice_new(starkbank_entity **out);
/* An empty Invoice to fill in for a create batch. Free it with
   starkbank_entity_free, or hand it to starkbank_list_append, which takes it. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_params_new(starkbank_entity **out);
/* A query or patch bag, tagged "Invoice.Params" and validated against the same
   table, so a mistyped filter fails here instead of being quietly ignored by
   the API. Build it with the same setters, free it with starkbank_entity_free.
   Build with -DSTARKBANK_LOOSE_QUERY to accept a filter this build's table has
   not learned yet: a wrong filter costs a retry where a wrong write costs
   money, so this loosening exists and the one on writes does not. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_create(const starkbank_client *client,
    const starkbank_list *invoices, starkbank_list **out, starkbank_errors **errors);
/* Up to 100 per call. Every entity is checked against the REQUIRED flags
   before anything is sent: a missing one returns STARKBANK_ERROR_FIELD and no
   request is made, rather than costing a round trip for a 400. Which key is
   missing is not reported back - this ABI has no channel to carry a name, and
   adding one is a decision deferred rather than a detail forgotten. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_invoice_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
/* python's generator. limit <= 0 walks every page. params may be NULL for no
   filters. Network and API failures surface from starkbank_iter_next. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);
/* One page, the (list, cursor) pair python returns. *out_cursor is NULL on the
   last page; when it is not, free it with starkbank_free and pass it back in
   params under "cursor" for the next call. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* Every key in patch must carry STARKBANK_FLAG_PATCH: amount, due, expiration,
   status. Build patch with starkbank_invoice_params_new. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Raw PDF bytes. Free with starkbank_free; *out is NULL on failure. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_qrcode(const starkbank_client *client,
    const char *id, int size, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Raw PNG bytes. size is pixels per box, 1..50; pass 0 to send no size at all
   and let the API apply its own default of 7. Free with starkbank_free. */

STARKBANK_API int STARKBANK_CALL starkbank_invoice_payment(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Available once the Invoice is paid; the entity is tagged "Invoice.Payment".
   Yours to free. */

/* --------------------------------------------------------- Invoice.Payment */
/* Return-only, reached through starkbank_invoice_payment.
 * Fields: amount AMOUNT (ro); name taxId bankCode branchCode accountNumber
 * accountType endToEndId method STRING (ro). */
#define STARKBANK_INVOICE_PAYMENT_AMOUNT          "amount"
#define STARKBANK_INVOICE_PAYMENT_NAME            "name"
#define STARKBANK_INVOICE_PAYMENT_TAX_ID          "taxId"
#define STARKBANK_INVOICE_PAYMENT_BANK_CODE       "bankCode"
#define STARKBANK_INVOICE_PAYMENT_BRANCH_CODE     "branchCode"
#define STARKBANK_INVOICE_PAYMENT_ACCOUNT_NUMBER  "accountNumber"
#define STARKBANK_INVOICE_PAYMENT_ACCOUNT_TYPE    "accountType"
#define STARKBANK_INVOICE_PAYMENT_END_TO_END_ID   "endToEndId"
#define STARKBANK_INVOICE_PAYMENT_METHOD          "method"

/* ------------------------------------------------------------ Invoice.Rule */
/* Modifies an Invoice's behaviour; passed in the "rules" list at create.
 * Fields: key* STRING, value* LIST_STRING.
 * ex: key "allowedTaxIds", value ["012.345.678-90", "45.059.493/0001-73"]. */
#define STARKBANK_INVOICE_RULE_KEY    "key"
#define STARKBANK_INVOICE_RULE_VALUE  "value"

STARKBANK_API int STARKBANK_CALL starkbank_invoice_rule_new(starkbank_entity **out);
/* Free it, or append it to an Invoice's "rules", which takes ownership. */

/* ---------------------------------------------------------------- Split */
/* Passed in an Invoice's "splits" list to name the payment's receivers.
 * Fields: amount* AMOUNT, receiverId* STRING, externalId STRING,
 * tags LIST_STRING, scheduled DATETIME; id source status STRING (ro),
 * created updated DATETIME (ro). */
#define STARKBANK_SPLIT_AMOUNT       "amount"
#define STARKBANK_SPLIT_RECEIVER_ID  "receiverId"
#define STARKBANK_SPLIT_EXTERNAL_ID  "externalId"
#define STARKBANK_SPLIT_TAGS         "tags"
#define STARKBANK_SPLIT_SCHEDULED    "scheduled"
#define STARKBANK_SPLIT_SOURCE       "source"
#define STARKBANK_SPLIT_STATUS       "status"
#define STARKBANK_SPLIT_ID           "id"
#define STARKBANK_SPLIT_CREATED      "created"
#define STARKBANK_SPLIT_UPDATED      "updated"

STARKBANK_API int STARKBANK_CALL starkbank_split_new(starkbank_entity **out);

/* ------------------------------------------------------------- invoice.Log */
/*
 * Resource "InvoiceLog"; the endpoint "invoice/log" is derived at run time by
 * starkcore_api_endpoint, never spelled here.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), invoice RESOURCE("Invoice") (ro).
 * Query keys: limit, after, before, types, invoiceIds.
 *
 * The invoice field is a whole tagged Invoice read with the same accessors,
 * which is the clearest argument for there being exactly one entity type.
 */
#define STARKBANK_INVOICE_LOG_ID       "id"
#define STARKBANK_INVOICE_LOG_CREATED  "created"
#define STARKBANK_INVOICE_LOG_TYPE     "type"
#define STARKBANK_INVOICE_LOG_ERRORS   "errors"
#define STARKBANK_INVOICE_LOG_INVOICE  "invoice"

STARKBANK_API int STARKBANK_CALL starkbank_invoice_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_invoice_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_invoice_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_invoice_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_invoice_log_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* A PDF of the Invoice as it stood at this log entry. Free with starkbank_free. */

/* =========================================================================
 *                                 Transfer
 * =========================================================================
 *
 * Fields (wire keys; * = required on create). Transfer has no update verb, so
 * nothing here is patchable.
 *
 *   amount* AMOUNT             name* STRING           taxId* STRING
 *   bankCode* STRING           branchCode* STRING     accountNumber* STRING
 *   accountType STRING         externalId STRING      scheduled DATE_OR_DATETIME
 *   description STRING         displayDescription STRING
 *   tags LIST_STRING           rules LIST_RESOURCE("Transfer.Rule")
 *   id status STRING (ro)      fee AMOUNT (ro)        transactionIds LIST_STRING (ro)
 *   metadata OBJECT (ro)       created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, transactionIds, status, taxId, sort, tags, ids.
 *
 * bankCode decides the rail: an 8-digit ISPB makes a Pix transfer, anything
 * else a TED. accountType only has an effect on Pix.
 *
 * externalId is the idempotency key, and it is the one every caller should
 * set: a url-safe string unique across all your transfers. Without it the API
 * blocks any transfer repeating the same amount and receiver on the same day,
 * which is a duplicate guard rather than idempotency, and a retried request
 * whose reply you never saw is indistinguishable from a second payment.
 */

#define STARKBANK_TRANSFER_AMOUNT               "amount"
#define STARKBANK_TRANSFER_NAME                 "name"
#define STARKBANK_TRANSFER_TAX_ID               "taxId"
#define STARKBANK_TRANSFER_BANK_CODE            "bankCode"
#define STARKBANK_TRANSFER_BRANCH_CODE          "branchCode"
#define STARKBANK_TRANSFER_ACCOUNT_NUMBER       "accountNumber"
#define STARKBANK_TRANSFER_ACCOUNT_TYPE         "accountType"
#define STARKBANK_TRANSFER_EXTERNAL_ID          "externalId"
#define STARKBANK_TRANSFER_SCHEDULED            "scheduled"
#define STARKBANK_TRANSFER_DESCRIPTION          "description"
#define STARKBANK_TRANSFER_DISPLAY_DESCRIPTION  "displayDescription"
#define STARKBANK_TRANSFER_TAGS                 "tags"
#define STARKBANK_TRANSFER_RULES                "rules"
#define STARKBANK_TRANSFER_FEE                  "fee"
#define STARKBANK_TRANSFER_STATUS               "status"
#define STARKBANK_TRANSFER_TRANSACTION_IDS      "transactionIds"
#define STARKBANK_TRANSFER_METADATA             "metadata"
#define STARKBANK_TRANSFER_ID                   "id"
#define STARKBANK_TRANSFER_CREATED              "created"
#define STARKBANK_TRANSFER_UPDATED              "updated"

#define STARKBANK_TRANSFER_ACCOUNT_TYPE_CHECKING  "checking"
#define STARKBANK_TRANSFER_ACCOUNT_TYPE_SAVINGS   "savings"
#define STARKBANK_TRANSFER_ACCOUNT_TYPE_SALARY    "salary"
#define STARKBANK_TRANSFER_ACCOUNT_TYPE_PAYMENT   "payment"

#define STARKBANK_TRANSFER_STATUS_CREATED    "created"
#define STARKBANK_TRANSFER_STATUS_PROCESSING "processing"
#define STARKBANK_TRANSFER_STATUS_SUCCESS    "success"
#define STARKBANK_TRANSFER_STATUS_FAILED     "failed"
#define STARKBANK_TRANSFER_STATUS_CANCELED   "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_transfer_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_transfer_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_transfer_create(const starkbank_client *client,
    const starkbank_list *transfers, starkbank_list **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_transfer_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_transfer_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Cancels a Transfer that has not been processed yet and returns it as it
   stands. Yours to free. */

STARKBANK_API int STARKBANK_CALL starkbank_transfer_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_transfer_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_transfer_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* The receipt, available only once the Transfer has succeeded. Free with
   starkbank_free. */

/* ----------------------------------------------------------- Transfer.Rule */
/* Fields: key* STRING, value* NUMBER. ex: key "resendingLimit", value 5.
   Note the value type: an Invoice.Rule value is a list of strings and a
   Transfer.Rule value is a number, which is why each sub-resource carries its
   own table and its own tag rather than sharing one "Rule". */
#define STARKBANK_TRANSFER_RULE_KEY    "key"
#define STARKBANK_TRANSFER_RULE_VALUE  "value"

STARKBANK_API int STARKBANK_CALL starkbank_transfer_rule_new(starkbank_entity **out);

/* ------------------------------------------------------------ transfer.Log */
/*
 * Resource "TransferLog"; endpoint "transfer/log", derived at run time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), transfer RESOURCE("Transfer") (ro).
 * Query keys: limit, after, before, types, transferIds.
 *
 * There is no transfer.Log pdf: sdk-python does not have one, and python is
 * normative for the verb surface. The receipt is starkbank_transfer_pdf.
 */
#define STARKBANK_TRANSFER_LOG_ID        "id"
#define STARKBANK_TRANSFER_LOG_CREATED   "created"
#define STARKBANK_TRANSFER_LOG_TYPE      "type"
#define STARKBANK_TRANSFER_LOG_ERRORS    "errors"
#define STARKBANK_TRANSFER_LOG_TRANSFER  "transfer"

STARKBANK_API int STARKBANK_CALL starkbank_transfer_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_transfer_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_transfer_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_transfer_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                  Event
 * =========================================================================
 *
 * A webhook notification. Events are never created by a caller.
 *
 * Fields (wire keys; + = accepted in an update):
 *   id STRING (ro)             created DATETIME (ro)  subscription STRING (ro)
 *   workspaceId STRING (ro)    isDelivered+ BOOL      log RESOURCE (ro)
 *
 * Query keys: limit, after, before, isDelivered.
 *
 * log is polymorphic: the resource it hydrates as is chosen by subscription -
 * "invoice" gives an InvoiceLog, "transfer" a TransferLog, and so on for the
 * ten subscriptions sdk-python maps. The tag on the entity you get back says
 * which, so read it with starkbank_entity_resource before reaching inside:
 *
 *     starkbank_entity_entity(event, STARKBANK_EVENT_LOG, &log);
 *     if (strcmp(starkbank_entity_resource(log), "InvoiceLog") == 0) {
 *         starkbank_entity_entity(log, STARKBANK_INVOICE_LOG_INVOICE, &invoice);
 *     }
 *
 * A subscription this build does not know still hydrates: the log entity is
 * untagged and permissive, everything in it is reachable, and the unknown
 * counter goes up so the gap shows in CI rather than in a caller's log.
 */

#define STARKBANK_EVENT_ID            "id"
#define STARKBANK_EVENT_LOG           "log"
#define STARKBANK_EVENT_CREATED       "created"
#define STARKBANK_EVENT_IS_DELIVERED  "isDelivered"
#define STARKBANK_EVENT_SUBSCRIPTION  "subscription"
#define STARKBANK_EVENT_WORKSPACE_ID  "workspaceId"

#define STARKBANK_EVENT_SUBSCRIPTION_TRANSFER        "transfer"
#define STARKBANK_EVENT_SUBSCRIPTION_INVOICE         "invoice"
#define STARKBANK_EVENT_SUBSCRIPTION_DEPOSIT         "deposit"
#define STARKBANK_EVENT_SUBSCRIPTION_BOLETO          "boleto"
#define STARKBANK_EVENT_SUBSCRIPTION_BRCODE_PAYMENT  "brcode-payment"
#define STARKBANK_EVENT_SUBSCRIPTION_BOLETO_PAYMENT  "boleto-payment"
#define STARKBANK_EVENT_SUBSCRIPTION_UTILITY_PAYMENT "utility-payment"
#define STARKBANK_EVENT_SUBSCRIPTION_DARF_PAYMENT    "darf-payment"
#define STARKBANK_EVENT_SUBSCRIPTION_TAX_PAYMENT     "tax-payment"
#define STARKBANK_EVENT_SUBSCRIPTION_HOLMES          "holmes"

STARKBANK_API int STARKBANK_CALL starkbank_event_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_event_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_event_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_event_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_event_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* isDelivered is the only patchable key; setting it to 1 stops redelivery. */

STARKBANK_API int STARKBANK_CALL starkbank_event_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_event_parse(const starkbank_client *client,
    const char *content, size_t content_len, const char *signature_base64,
    starkbank_entity **out, starkbank_errors **errors);
/*
 * The one verb in this slice with judgement in it, and therefore the one no
 * macro expands: it is declared here and defined by hand in handwritten/. If
 * nobody writes that file the link fails with an undefined symbol, which is
 * the only marker for "this needs a human" that cannot be ignored.
 *
 * Verifies the body against Stark's public key and hydrates the Event inside
 * the "event" envelope. STARKCORE_ERROR_SIGNATURE when it does not check out;
 * do not trust the body in that case. Untrusted input, and fuzzed as such.
 */

/* --------------------------------------------------------- event.Attempt */
/*
 * A failed delivery of an Event, recorded so a caller can debug their own
 * endpoint. Resource "EventAttempt"; endpoint "event/attempt".
 * Fields: id code message eventId webhookId STRING (ro), created DATETIME (ro).
 * Query keys: limit, after, before, eventIds, webhookIds.
 */
#define STARKBANK_EVENT_ATTEMPT_ID          "id"
#define STARKBANK_EVENT_ATTEMPT_CODE        "code"
#define STARKBANK_EVENT_ATTEMPT_MESSAGE     "message"
#define STARKBANK_EVENT_ATTEMPT_EVENT_ID    "eventId"
#define STARKBANK_EVENT_ATTEMPT_WEBHOOK_ID  "webhookId"
#define STARKBANK_EVENT_ATTEMPT_CREATED     "created"

STARKBANK_API int STARKBANK_CALL starkbank_event_attempt_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_event_attempt_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_event_attempt_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_event_attempt_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                 Balance
 * =========================================================================
 *
 * The degenerate shape, and worth having in the first slice for exactly that:
 * a single object with no id, no filters and no verbs but one.
 *
 * Fields: id STRING (ro), amount AMOUNT (ro), currency STRING (ro),
 *         updated DATETIME (ro).
 */
#define STARKBANK_BALANCE_ID        "id"
#define STARKBANK_BALANCE_AMOUNT    "amount"
#define STARKBANK_BALANCE_CURRENCY  "currency"
#define STARKBANK_BALANCE_UPDATED   "updated"

STARKBANK_API int STARKBANK_CALL starkbank_balance_get(const starkbank_client *client,
    starkbank_entity **out, starkbank_errors **errors);
/* The workspace's balance. There is no id: python takes the first element of
   the listing endpoint, and so does this. Yours to free. */

/* =========================================================================
 *                                 Webhook
 * =========================================================================
 *
 * A subscription: the URL we POST an Event to, and which services it hears
 * about. The delivered body is verified with starkbank_event_parse.
 *
 * Fields (wire keys; * = required on create).
 *
 *   url* STRING                subscriptions* LIST_STRING
 *   id STRING (ro)
 *
 * Query keys: limit.
 *
 * Webhook is the one bank resource created ONE AT A TIME, so
 * starkbank_webhook_create takes an entity where every other create takes a
 * list. The body on the wire is that entity's own object: python's
 * rest.post_single sends api_json(entity) with no wrapper, and the singular
 * key "webhook" appears only in the response, which core-c unwraps. Neither
 * tier ever sends {"webhooks": [...]} for this verb.
 *
 * A delivery that does not answer 200 is retried at 5, 30 and 120 minutes and
 * then dropped, so a daily starkbank_event_query with isDelivered false is
 * part of using this resource rather than an optional extra.
 */
#define STARKBANK_WEBHOOK_URL            "url"
#define STARKBANK_WEBHOOK_SUBSCRIPTIONS  "subscriptions"
#define STARKBANK_WEBHOOK_ID             "id"

/* The services a subscription may name, from sdk-python's own list. Strings
   for the reason every vocabulary here is a string: the API grows one without
   asking us, and an unknown string is merely unrecognised. */
#define STARKBANK_WEBHOOK_SUBSCRIPTION_TRANSFER        "transfer"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_INVOICE         "invoice"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_DEPOSIT         "deposit"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_BOLETO          "boleto"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_BOLETO_HOLMES   "boleto-holmes"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_BOLETO_PAYMENT  "boleto-payment"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_BRCODE_PAYMENT  "brcode-payment"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_UTILITY_PAYMENT "utility-payment"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_DARF_PAYMENT    "darf-payment"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_TAX_PAYMENT     "tax-payment"
#define STARKBANK_WEBHOOK_SUBSCRIPTION_PAYMENT_REQUEST "payment-request"

STARKBANK_API int STARKBANK_CALL starkbank_webhook_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_webhook_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_webhook_create(const starkbank_client *client,
    const starkbank_entity *webhook, starkbank_entity **out, starkbank_errors **errors);
/* rest.post_single: one Webhook, sent as the body itself. The entity stays
   yours - this does not take ownership the way starkbank_list_append does -
   and the Webhook that comes back is a second entity, also yours to free. */

STARKBANK_API int STARKBANK_CALL starkbank_webhook_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_webhook_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_webhook_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_webhook_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Deletion cannot be undone: the subscription stops receiving events. */

/* =========================================================================
 *                             PaymentPreview
 * =========================================================================
 *
 * What a payment code says before you pay it. The code goes in as id - a BR
 * Code for Pix, a line or a bar code for a slip - and what comes back is the
 * preview the code turned out to describe.
 *
 * Fields (wire keys; * = required on create).
 *
 *   id* STRING                 scheduled DATE
 *   type STRING (ro)           payment RESOURCE (ro).
 *
 * payment is polymorphic: which table it hydrates as is chosen by the sibling
 * type field, exactly as Event.log is chosen by subscription, and the mapping
 * is sdk-python's _sub_resource_by_type -
 *
 *   "brcode-payment"  -> PaymentPreview.BrcodePreview
 *   "boleto-payment"  -> PaymentPreview.BoletoPreview
 *   "utility-payment" -> PaymentPreview.UtilityPreview
 *   "tax-payment"     -> PaymentPreview.TaxPreview
 *
 * A type this build predates leaves payment untagged: still readable through
 * the accessors, still dumped, and counted by starkbank_entity_unknown_count
 * as exactly one unknown.
 *
 * scheduled is a date and only affects a BrcodePreview, where the amount a Pix
 * charge asks for depends on the day it is paid.
 */
#define STARKBANK_PAYMENT_PREVIEW_ID         "id"
#define STARKBANK_PAYMENT_PREVIEW_SCHEDULED  "scheduled"
#define STARKBANK_PAYMENT_PREVIEW_TYPE       "type"
#define STARKBANK_PAYMENT_PREVIEW_PAYMENT    "payment"

#define STARKBANK_PAYMENT_PREVIEW_TYPE_BRCODE_PAYMENT  "brcode-payment"
#define STARKBANK_PAYMENT_PREVIEW_TYPE_BOLETO_PAYMENT  "boleto-payment"
#define STARKBANK_PAYMENT_PREVIEW_TYPE_UTILITY_PAYMENT "utility-payment"
#define STARKBANK_PAYMENT_PREVIEW_TYPE_TAX_PAYMENT     "tax-payment"

STARKBANK_API int STARKBANK_CALL starkbank_payment_preview_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_payment_preview_create(
    const starkbank_client *client, const starkbank_list *previews,
    starkbank_list **out, starkbank_errors **errors);
/* rest.post_multi. A batch may mix types freely: each preview in the reply
   resolves its own payment table from its own type. */

/* ------------------------------------------- PaymentPreview.BrcodePreview */
/*
 * Fields: status name taxId bankCode accountType reconciliationId description
 *         STRING (ro); allowChange BOOL (ro); amount nominalAmount
 *         interestAmount fineAmount reductionAmount discountAmount AMOUNT (ro).
 *
 * amount 0 means the charge accepts any amount, and allowChange says whether
 * the payer may send something other than amount - so 0 and absent are
 * different answers here, which is why no accessor invents a default.
 */
#define STARKBANK_BRCODE_PREVIEW_STATUS            "status"
#define STARKBANK_BRCODE_PREVIEW_NAME              "name"
#define STARKBANK_BRCODE_PREVIEW_TAX_ID            "taxId"
#define STARKBANK_BRCODE_PREVIEW_BANK_CODE         "bankCode"
#define STARKBANK_BRCODE_PREVIEW_ACCOUNT_TYPE      "accountType"
#define STARKBANK_BRCODE_PREVIEW_ALLOW_CHANGE      "allowChange"
#define STARKBANK_BRCODE_PREVIEW_AMOUNT            "amount"
#define STARKBANK_BRCODE_PREVIEW_NOMINAL_AMOUNT    "nominalAmount"
#define STARKBANK_BRCODE_PREVIEW_INTEREST_AMOUNT   "interestAmount"
#define STARKBANK_BRCODE_PREVIEW_FINE_AMOUNT       "fineAmount"
#define STARKBANK_BRCODE_PREVIEW_REDUCTION_AMOUNT  "reductionAmount"
#define STARKBANK_BRCODE_PREVIEW_DISCOUNT_AMOUNT   "discountAmount"
#define STARKBANK_BRCODE_PREVIEW_RECONCILIATION_ID "reconciliationId"
#define STARKBANK_BRCODE_PREVIEW_DESCRIPTION       "description"

/* ------------------------------------------- PaymentPreview.BoletoPreview */
/*
 * Fields: status due expiration name taxId receiverName receiverTaxId
 *         payerName payerTaxId line barCode STRING (ro);
 *         amount discountAmount fineAmount interestAmount AMOUNT (ro).
 *
 * due and expiration are dates on the wire and STRING here, because
 * sdk-python's BoletoPreview performs no check_date on them and hands its own
 * caller the string the API sent. Declaring a coercion python does not make
 * would be this table inventing a semantic, which tools/drift.py treats as a
 * hard stop rather than a preference.
 */
#define STARKBANK_BOLETO_PREVIEW_STATUS           "status"
#define STARKBANK_BOLETO_PREVIEW_AMOUNT           "amount"
#define STARKBANK_BOLETO_PREVIEW_DISCOUNT_AMOUNT  "discountAmount"
#define STARKBANK_BOLETO_PREVIEW_FINE_AMOUNT      "fineAmount"
#define STARKBANK_BOLETO_PREVIEW_INTEREST_AMOUNT  "interestAmount"
#define STARKBANK_BOLETO_PREVIEW_DUE              "due"
#define STARKBANK_BOLETO_PREVIEW_EXPIRATION       "expiration"
#define STARKBANK_BOLETO_PREVIEW_NAME             "name"
#define STARKBANK_BOLETO_PREVIEW_TAX_ID           "taxId"
#define STARKBANK_BOLETO_PREVIEW_RECEIVER_NAME    "receiverName"
#define STARKBANK_BOLETO_PREVIEW_RECEIVER_TAX_ID  "receiverTaxId"
#define STARKBANK_BOLETO_PREVIEW_PAYER_NAME       "payerName"
#define STARKBANK_BOLETO_PREVIEW_PAYER_TAX_ID     "payerTaxId"
#define STARKBANK_BOLETO_PREVIEW_LINE             "line"
#define STARKBANK_BOLETO_PREVIEW_BAR_CODE         "barCode"

/* ---------------------------------------------- PaymentPreview.TaxPreview */
/* Fields: name description line barCode STRING (ro); amount AMOUNT (ro). */
#define STARKBANK_TAX_PREVIEW_AMOUNT       "amount"
#define STARKBANK_TAX_PREVIEW_NAME         "name"
#define STARKBANK_TAX_PREVIEW_DESCRIPTION  "description"
#define STARKBANK_TAX_PREVIEW_LINE         "line"
#define STARKBANK_TAX_PREVIEW_BAR_CODE     "barCode"

/* ------------------------------------------ PaymentPreview.UtilityPreview */
/* Fields: name description line barCode STRING (ro); amount AMOUNT (ro). */
#define STARKBANK_UTILITY_PREVIEW_AMOUNT       "amount"
#define STARKBANK_UTILITY_PREVIEW_NAME         "name"
#define STARKBANK_UTILITY_PREVIEW_DESCRIPTION  "description"
#define STARKBANK_UTILITY_PREVIEW_LINE         "line"
#define STARKBANK_UTILITY_PREVIEW_BAR_CODE     "barCode"

/* =========================================================================
 *                                 Boleto
 * =========================================================================
 *
 * Fields (wire keys; * = required on create). Boleto has no update verb, so
 * nothing here is patchable.
 *
 *   amount* AMOUNT
 *   name* taxId* streetLine1* streetLine2* district* city* stateCode*
 *   zipCode* STRING
 *   due DATE                fine interest RATE          overdueLimit NUMBER
 *   descriptions discounts LIST_OBJECT                  tags LIST_STRING
 *   receiverName receiverTaxId STRING
 *   fee AMOUNT (ro)
 *   line barCode status STRING (ro)
 *   transactionIds LIST_STRING (ro)
 *   workspaceId ourNumber id STRING (ro)                created DATETIME (ro)
 *
 * Query keys: limit, after, before, status, tags, ids.
 *
 * due is a plain DATE: unlike Invoice.due, sdk-python calls check_date, never
 * check_datetime_or_date, and there is no scheduled-boleto equivalent. pdf
 * takes an optional layout ("default"/"booklet") and an optional list of
 * field names to hide, both sent only when non-empty.
 */

#define STARKBANK_BOLETO_AMOUNT           "amount"
#define STARKBANK_BOLETO_NAME             "name"
#define STARKBANK_BOLETO_TAX_ID           "taxId"
#define STARKBANK_BOLETO_STREET_LINE_1    "streetLine1"
#define STARKBANK_BOLETO_STREET_LINE_2    "streetLine2"
#define STARKBANK_BOLETO_DISTRICT         "district"
#define STARKBANK_BOLETO_CITY             "city"
#define STARKBANK_BOLETO_STATE_CODE       "stateCode"
#define STARKBANK_BOLETO_ZIP_CODE         "zipCode"
#define STARKBANK_BOLETO_DUE              "due"
#define STARKBANK_BOLETO_FINE             "fine"
#define STARKBANK_BOLETO_INTEREST         "interest"
#define STARKBANK_BOLETO_OVERDUE_LIMIT    "overdueLimit"
#define STARKBANK_BOLETO_DESCRIPTIONS     "descriptions"
#define STARKBANK_BOLETO_DISCOUNTS        "discounts"
#define STARKBANK_BOLETO_TAGS             "tags"
#define STARKBANK_BOLETO_RECEIVER_NAME    "receiverName"
#define STARKBANK_BOLETO_RECEIVER_TAX_ID  "receiverTaxId"
#define STARKBANK_BOLETO_FEE              "fee"
#define STARKBANK_BOLETO_LINE             "line"
#define STARKBANK_BOLETO_BAR_CODE         "barCode"
#define STARKBANK_BOLETO_STATUS           "status"
#define STARKBANK_BOLETO_TRANSACTION_IDS  "transactionIds"
#define STARKBANK_BOLETO_WORKSPACE_ID     "workspaceId"
#define STARKBANK_BOLETO_OUR_NUMBER       "ourNumber"
#define STARKBANK_BOLETO_ID               "id"
#define STARKBANK_BOLETO_CREATED          "created"

/* app-docs' list; sdk-python's own docstring example also shows "registered",
   which the docs list omits - the same gap design.md records for Invoice. */
#define STARKBANK_BOLETO_STATUS_CREATED     "created"
#define STARKBANK_BOLETO_STATUS_REGISTERED  "registered"
#define STARKBANK_BOLETO_STATUS_OVERDUE     "overdue"
#define STARKBANK_BOLETO_STATUS_PAID        "paid"
#define STARKBANK_BOLETO_STATUS_CANCELED    "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_boleto_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_create(const starkbank_client *client,
    const starkbank_list *boletos, starkbank_list **out, starkbank_errors **errors);
/* Up to 100 per call. If a Boleto is paid after its due date with a fine, an
   interest or a discount applied, the returned amount reflects what was
   actually paid, not what was requested. */

STARKBANK_API int STARKBANK_CALL starkbank_boleto_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Sends a cancellation request to CIP; once canceled a Boleto can no longer be
   paid. This cannot be undone. */

STARKBANK_API int STARKBANK_CALL starkbank_boleto_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_pdf(const starkbank_client *client,
    const char *id, const char *layout, const char *hidden_fields,
    unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* layout is "default" or "booklet"; pass NULL to send neither. hidden_fields
   names fields to omit from the rendered PDF, comma-joined, ex:
   "customerAddress,customerTaxId"; pass NULL to send none - core-c's query
   encoder percent-encodes the whole value, so a caller-joined string and
   sdk-python's list of the same names reach the wire as the same bytes. This
   route is public and needs no authentication, but the API blocks the
   caller's IP after repeated requests for invalid ids. Free the bytes with
   starkbank_free. */

/* --------------------------------------------------------------- BoletoLog */
/*
 * Resource "BoletoLog"; endpoint "boleto/log", derived at run time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), boleto RESOURCE("Boleto") (ro).
 * Query keys: limit, after, before, types, boletoIds.
 *
 * There is no boleto.Log pdf: sdk-python does not have one, and python is
 * normative for the verb surface. The receipt is starkbank_boleto_pdf.
 */
#define STARKBANK_BOLETO_LOG_ID       "id"
#define STARKBANK_BOLETO_LOG_CREATED  "created"
#define STARKBANK_BOLETO_LOG_TYPE     "type"
#define STARKBANK_BOLETO_LOG_ERRORS   "errors"
#define STARKBANK_BOLETO_LOG_BOLETO   "boleto"

STARKBANK_API int STARKBANK_CALL starkbank_boleto_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                              BoletoPayment
 * =========================================================================
 *
 * Fields (wire keys; * = required on create).
 *
 *   taxId* description* STRING     line barCode STRING
 *   amount AMOUNT                  scheduled DATE       tags LIST_STRING
 *   id status STRING (ro)          fee AMOUNT (ro)
 *   transactionIds LIST_STRING (ro)                     created DATETIME (ro)
 *
 * Query keys: limit, after, before, tags, ids, status.
 *
 * Either line or barCode identifies the boleto being paid; sdk-python calls
 * both "conditionally required" in prose rather than in its signature, so
 * neither carries REQUIRED here - a caller who omits both is told by the API.
 */

#define STARKBANK_BOLETO_PAYMENT_TAX_ID          "taxId"
#define STARKBANK_BOLETO_PAYMENT_DESCRIPTION     "description"
#define STARKBANK_BOLETO_PAYMENT_LINE            "line"
#define STARKBANK_BOLETO_PAYMENT_BAR_CODE        "barCode"
#define STARKBANK_BOLETO_PAYMENT_AMOUNT          "amount"
#define STARKBANK_BOLETO_PAYMENT_SCHEDULED       "scheduled"
#define STARKBANK_BOLETO_PAYMENT_TAGS            "tags"
#define STARKBANK_BOLETO_PAYMENT_ID              "id"
#define STARKBANK_BOLETO_PAYMENT_STATUS          "status"
#define STARKBANK_BOLETO_PAYMENT_FEE             "fee"
#define STARKBANK_BOLETO_PAYMENT_TRANSACTION_IDS "transactionIds"
#define STARKBANK_BOLETO_PAYMENT_CREATED         "created"

#define STARKBANK_BOLETO_PAYMENT_STATUS_CREATED     "created"
#define STARKBANK_BOLETO_PAYMENT_STATUS_PROCESSING  "processing"
#define STARKBANK_BOLETO_PAYMENT_STATUS_CONFIRMED   "confirmed"
#define STARKBANK_BOLETO_PAYMENT_STATUS_SUCCESS     "success"
#define STARKBANK_BOLETO_PAYMENT_STATUS_FAILED      "failed"
#define STARKBANK_BOLETO_PAYMENT_STATUS_CANCELED    "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_create(const starkbank_client *client,
    const starkbank_list *payments, starkbank_list **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Cancels a payment that has not started processing; a payment already
   processed can still be deleted, but the payment itself is not reversed. */

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Valid only once the payment carries status "success", "processing" or
   "created". Free with starkbank_free. */

/* ------------------------------------------------------------ BoletoPaymentLog */
/*
 * Resource "BoletoPaymentLog"; endpoint "boleto-payment/log", derived at run
 * time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), payment RESOURCE("BoletoPayment") (ro).
 * Query keys: limit, after, before, types, paymentIds.
 *
 * There is no boletopayment.Log pdf: sdk-python does not have one, and python
 * is normative for the verb surface. The receipt is starkbank_boleto_payment_pdf.
 */
#define STARKBANK_BOLETO_PAYMENT_LOG_ID       "id"
#define STARKBANK_BOLETO_PAYMENT_LOG_CREATED  "created"
#define STARKBANK_BOLETO_PAYMENT_LOG_TYPE     "type"
#define STARKBANK_BOLETO_PAYMENT_LOG_ERRORS   "errors"
#define STARKBANK_BOLETO_PAYMENT_LOG_PAYMENT  "payment"

STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_boleto_payment_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                              BrcodePayment
 * =========================================================================
 *
 * Fields (wire keys; * = required on create, + = also accepted in an update).
 *
 *   brcode* taxId* description* STRING
 *   amount AMOUNT           scheduled DATE          tags LIST_STRING
 *   rules LIST_RESOURCE("BrcodePayment.Rule")
 *   id name type STRING (ro)                        status+ STRING
 *   transactionIds LIST_STRING (ro)                 fee AMOUNT (ro)
 *   updated created DATETIME (ro)
 *
 * Query keys: limit, after, before, tags, ids, status.
 *
 * status is PATCH and neither CREATE nor REQUIRED: sdk-python's update()
 * accepts only status, to cancel a payment before it is processed - the same
 * shape Invoice.status has, and for the same reason.
 *
 * scheduled is DATE: sdk-python calls check_date despite a docstring that
 * still reads "date, datetime.datetime or string" - the coercion is
 * normative, not the prose beside it.
 */

#define STARKBANK_BRCODE_PAYMENT_BRCODE           "brcode"
#define STARKBANK_BRCODE_PAYMENT_TAX_ID           "taxId"
#define STARKBANK_BRCODE_PAYMENT_DESCRIPTION      "description"
#define STARKBANK_BRCODE_PAYMENT_AMOUNT           "amount"
#define STARKBANK_BRCODE_PAYMENT_SCHEDULED        "scheduled"
#define STARKBANK_BRCODE_PAYMENT_TAGS             "tags"
#define STARKBANK_BRCODE_PAYMENT_RULES            "rules"
#define STARKBANK_BRCODE_PAYMENT_ID               "id"
#define STARKBANK_BRCODE_PAYMENT_NAME             "name"
#define STARKBANK_BRCODE_PAYMENT_STATUS           "status"
#define STARKBANK_BRCODE_PAYMENT_TYPE             "type"
#define STARKBANK_BRCODE_PAYMENT_TRANSACTION_IDS  "transactionIds"
#define STARKBANK_BRCODE_PAYMENT_FEE              "fee"
#define STARKBANK_BRCODE_PAYMENT_UPDATED          "updated"
#define STARKBANK_BRCODE_PAYMENT_CREATED          "created"

#define STARKBANK_BRCODE_PAYMENT_STATUS_CREATED     "created"
#define STARKBANK_BRCODE_PAYMENT_STATUS_PROCESSING  "processing"
#define STARKBANK_BRCODE_PAYMENT_STATUS_CONFIRMED   "confirmed"
#define STARKBANK_BRCODE_PAYMENT_STATUS_SUCCESS     "success"
#define STARKBANK_BRCODE_PAYMENT_STATUS_FAILED      "failed"
#define STARKBANK_BRCODE_PAYMENT_STATUS_CANCELED    "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_create(const starkbank_client *client,
    const starkbank_list *payments, starkbank_list **out, starkbank_errors **errors);
/* Processing is asynchronous: a freshly created BrcodePayment's amount is
   initially zero. */

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* status is the only patchable key; the only legal value is "canceled", and
   only before the payment has been paid. */

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Valid only once the payment carries status "success", "processing" or
   "created". Free with starkbank_free. There is no starkbank_brcode_payment_delete:
   sdk-python has no delete() for this resource, cancellation is the update above. */

/* ------------------------------------------------------- BrcodePayment.Rule */
/* Modifies a BrcodePayment's behaviour; passed in the "rules" list at create.
 * Fields: key* STRING, value* NUMBER. ex: key "resendingLimit", value 5.
 * Note the value type: a BrcodePayment.Rule value is a number, like
 * Transfer.Rule's and unlike Invoice.Rule's list of strings. */
#define STARKBANK_BRCODE_PAYMENT_RULE_KEY    "key"
#define STARKBANK_BRCODE_PAYMENT_RULE_VALUE  "value"

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_rule_new(starkbank_entity **out);

/* ------------------------------------------------------ BrcodePaymentLog */
/*
 * Resource "BrcodePaymentLog"; endpoint "brcode-payment/log", derived at run
 * time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), payment RESOURCE("BrcodePayment") (ro).
 * Query keys: limit, after, before, types, paymentIds.
 *
 * There is no brcodepayment.Log pdf: sdk-python does not have one, and python
 * is normative for the verb surface. The receipt is starkbank_brcode_payment_pdf.
 */
#define STARKBANK_BRCODE_PAYMENT_LOG_ID       "id"
#define STARKBANK_BRCODE_PAYMENT_LOG_CREATED  "created"
#define STARKBANK_BRCODE_PAYMENT_LOG_TYPE     "type"
#define STARKBANK_BRCODE_PAYMENT_LOG_ERRORS   "errors"
#define STARKBANK_BRCODE_PAYMENT_LOG_PAYMENT  "payment"

STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_brcode_payment_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                              UtilityPayment
 * =========================================================================
 *
 * Pay a utility bill (electricity, water, gas, ...) by its line or bar code.
 * UtilityPayment has no update verb, so nothing here is patchable.
 *
 * Fields (wire keys; * = required on create).
 *
 *   description* STRING
 *   line barCode STRING          scheduled DATE          tags LIST_STRING
 *   id status type STRING (ro)   amount fee AMOUNT (ro)
 *   transactionIds LIST_STRING (ro)      created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, tags, ids, status.
 *
 * line and barCode are the conditionally-required pair - exactly one
 * identifies the bill being paid, and if both are sent they must agree. As
 * with BoletoPayment's identical pair, python gives neither a positional
 * slot, so neither carries REQUIRED; a caller who gets the pairing wrong is
 * told by the API. scheduled is a plain DATE (check_date only, no
 * scheduled-invoice-shaped alternate meaning), defaulting to today.
 */
#define STARKBANK_UTILITY_PAYMENT_DESCRIPTION     "description"
#define STARKBANK_UTILITY_PAYMENT_LINE            "line"
#define STARKBANK_UTILITY_PAYMENT_BAR_CODE        "barCode"
#define STARKBANK_UTILITY_PAYMENT_SCHEDULED       "scheduled"
#define STARKBANK_UTILITY_PAYMENT_TAGS            "tags"
#define STARKBANK_UTILITY_PAYMENT_ID              "id"
#define STARKBANK_UTILITY_PAYMENT_STATUS          "status"
#define STARKBANK_UTILITY_PAYMENT_AMOUNT          "amount"
#define STARKBANK_UTILITY_PAYMENT_FEE             "fee"
#define STARKBANK_UTILITY_PAYMENT_TYPE            "type"
#define STARKBANK_UTILITY_PAYMENT_TRANSACTION_IDS "transactionIds"
#define STARKBANK_UTILITY_PAYMENT_CREATED         "created"
#define STARKBANK_UTILITY_PAYMENT_UPDATED         "updated"

#define STARKBANK_UTILITY_PAYMENT_STATUS_CREATED    "created"
#define STARKBANK_UTILITY_PAYMENT_STATUS_PROCESSING "processing"
#define STARKBANK_UTILITY_PAYMENT_STATUS_CONFIRMED  "confirmed"
#define STARKBANK_UTILITY_PAYMENT_STATUS_SUCCESS    "success"
#define STARKBANK_UTILITY_PAYMENT_STATUS_FAILED     "failed"
#define STARKBANK_UTILITY_PAYMENT_STATUS_CANCELED   "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_create(const starkbank_client *client,
    const starkbank_list *payments, starkbank_list **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Only cancels a payment that has not started processing yet; a payment
   already processed can still be deleted but is not reversed, as python's
   docstring says. Yours to free. */

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Only valid for a payment with "success", "processing" or "created" status,
   as python's docstring says. Free with starkbank_free. */

/* ------------------------------------------------------ UtilityPaymentLog */
/*
 * Resource "UtilityPaymentLog"; endpoint "utility-payment/log", derived at
 * run time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), payment RESOURCE("UtilityPayment") (ro).
 * Query keys: limit, after, before, types, paymentIds.
 *
 * There is no utilitypayment.Log pdf: sdk-python does not have one, and
 * python is normative for the verb surface. The receipt is
 * starkbank_utility_payment_pdf.
 */
#define STARKBANK_UTILITY_PAYMENT_LOG_ID       "id"
#define STARKBANK_UTILITY_PAYMENT_LOG_CREATED  "created"
#define STARKBANK_UTILITY_PAYMENT_LOG_TYPE     "type"
#define STARKBANK_UTILITY_PAYMENT_LOG_ERRORS   "errors"
#define STARKBANK_UTILITY_PAYMENT_LOG_PAYMENT  "payment"

STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_utility_payment_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                TaxPayment
 * =========================================================================
 *
 * Pay a tax slip (ISS, DAS, ...) by its line or bar code. TaxPayment has no
 * update verb, so nothing here is patchable.
 *
 * Fields (wire keys; * = required on create).
 *
 *   description* STRING
 *   line barCode STRING          scheduled DATE          tags LIST_STRING
 *   id type status STRING (ro)   amount fee AMOUNT (ro)
 *   transactionIds LIST_STRING (ro)      updated created DATETIME (ro)
 *
 * Query keys: limit, after, before, tags, ids, status.
 *
 * line and barCode are the conditionally-required pair, exactly as on
 * UtilityPayment: exactly one identifies the slip, and if both are sent they
 * must agree; neither carries REQUIRED because neither has a positional
 * slot in sdk-python's __init__. scheduled is a plain DATE (check_date only),
 * defaulting to today.
 */
#define STARKBANK_TAX_PAYMENT_DESCRIPTION     "description"
#define STARKBANK_TAX_PAYMENT_LINE            "line"
#define STARKBANK_TAX_PAYMENT_BAR_CODE        "barCode"
#define STARKBANK_TAX_PAYMENT_SCHEDULED       "scheduled"
#define STARKBANK_TAX_PAYMENT_TAGS            "tags"
#define STARKBANK_TAX_PAYMENT_ID              "id"
#define STARKBANK_TAX_PAYMENT_TYPE            "type"
#define STARKBANK_TAX_PAYMENT_STATUS          "status"
#define STARKBANK_TAX_PAYMENT_AMOUNT          "amount"
#define STARKBANK_TAX_PAYMENT_FEE             "fee"
#define STARKBANK_TAX_PAYMENT_TRANSACTION_IDS "transactionIds"
#define STARKBANK_TAX_PAYMENT_UPDATED         "updated"
#define STARKBANK_TAX_PAYMENT_CREATED         "created"

#define STARKBANK_TAX_PAYMENT_STATUS_CREATED    "created"
#define STARKBANK_TAX_PAYMENT_STATUS_PROCESSING "processing"
#define STARKBANK_TAX_PAYMENT_STATUS_CONFIRMED  "confirmed"
#define STARKBANK_TAX_PAYMENT_STATUS_SUCCESS    "success"
#define STARKBANK_TAX_PAYMENT_STATUS_FAILED     "failed"
#define STARKBANK_TAX_PAYMENT_STATUS_CANCELED   "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_create(const starkbank_client *client,
    const starkbank_list *payments, starkbank_list **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Only cancels a payment that has not started processing yet; a payment
   already processed can still be deleted but is not reversed. Yours to
   free. */

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Only valid for a payment with "success", "processing" or "created" status.
   Free with starkbank_free. */

/* ------------------------------------------------------------ TaxPaymentLog */
/*
 * Resource "TaxPaymentLog"; endpoint "tax-payment/log", derived at run time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), payment RESOURCE("TaxPayment") (ro).
 * Query keys: limit, after, before, types, paymentIds.
 *
 * There is no taxpayment.Log pdf: sdk-python does not have one, and python
 * is normative for the verb surface. The receipt is starkbank_tax_payment_pdf.
 */
#define STARKBANK_TAX_PAYMENT_LOG_ID       "id"
#define STARKBANK_TAX_PAYMENT_LOG_CREATED  "created"
#define STARKBANK_TAX_PAYMENT_LOG_TYPE     "type"
#define STARKBANK_TAX_PAYMENT_LOG_ERRORS   "errors"
#define STARKBANK_TAX_PAYMENT_LOG_PAYMENT  "payment"

STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_tax_payment_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                              CorporateCard
 * =========================================================================
 *
 * create() posts to "corporate-card/token", not "corporate-card" - see
 * starkbankVerbCreateSub in starkc/verb.c and the header comment in
 * starkbank/corporatecard/corporatecard.h for why that needs its own verb
 * macro, STARKBANK_VERB_POST_SINGLE_SUB, rather than STARKBANK_VERB_POST_SINGLE.
 *
 * Fields (wire keys; * = required on create, + = also accepted in an update).
 *
 *   holderId*  STRING
 *   holderName streetLine1 streetLine2 district city stateCode zipCode
 *     type number securityCode STRING (ro)
 *   displayName+ status+ STRING
 *   rules+ LIST_RESOURCE("CorporateRule")             tags+ LIST_STRING
 *   pin+ STRING
 *   expiration updated created DATETIME (ro)
 *   id STRING (ro).
 *
 * Query keys: limit, after, before, status, types, holderIds, ids, tags, expand.
 *
 * pin has no field in sdk-python's CorporateCard class - update() sends it
 * straight from a keyword argument and nothing ever reads it back - so it
 * carries PATCH and nothing else; see known-drift.json's
 * field.gone:CorporateCard:pin.
 *
 * number, securityCode and expiration are masked unless a caller passes
 * expand, which create() and get() do not yet forward for the reason
 * CorporateHolder's section explains; query() and page() do, through the
 * "expand" query key above.
 */

#define STARKBANK_CORPORATE_CARD_HOLDER_ID      "holderId"
#define STARKBANK_CORPORATE_CARD_HOLDER_NAME    "holderName"
#define STARKBANK_CORPORATE_CARD_DISPLAY_NAME   "displayName"
#define STARKBANK_CORPORATE_CARD_RULES          "rules"
#define STARKBANK_CORPORATE_CARD_TAGS           "tags"
#define STARKBANK_CORPORATE_CARD_PIN            "pin"
#define STARKBANK_CORPORATE_CARD_STREET_LINE_1  "streetLine1"
#define STARKBANK_CORPORATE_CARD_STREET_LINE_2  "streetLine2"
#define STARKBANK_CORPORATE_CARD_DISTRICT       "district"
#define STARKBANK_CORPORATE_CARD_CITY           "city"
#define STARKBANK_CORPORATE_CARD_STATE_CODE     "stateCode"
#define STARKBANK_CORPORATE_CARD_ZIP_CODE       "zipCode"
#define STARKBANK_CORPORATE_CARD_TYPE           "type"
#define STARKBANK_CORPORATE_CARD_STATUS         "status"
#define STARKBANK_CORPORATE_CARD_NUMBER         "number"
#define STARKBANK_CORPORATE_CARD_SECURITY_CODE  "securityCode"
#define STARKBANK_CORPORATE_CARD_EXPIRATION     "expiration"
#define STARKBANK_CORPORATE_CARD_ID             "id"
#define STARKBANK_CORPORATE_CARD_UPDATED        "updated"
#define STARKBANK_CORPORATE_CARD_CREATED        "created"

/* Types and statuses, from the docs' enums. */
#define STARKBANK_CORPORATE_CARD_TYPE_VIRTUAL   "virtual"
#define STARKBANK_CORPORATE_CARD_TYPE_PHYSICAL  "physical"
#define STARKBANK_CORPORATE_CARD_TYPE_WALLET    "wallet"

#define STARKBANK_CORPORATE_CARD_STATUS_PENDING  "pending"
#define STARKBANK_CORPORATE_CARD_STATUS_ACTIVE   "active"
#define STARKBANK_CORPORATE_CARD_STATUS_BLOCKED  "blocked"
#define STARKBANK_CORPORATE_CARD_STATUS_EXPIRED  "expired"
#define STARKBANK_CORPORATE_CARD_STATUS_CANCELED "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_create(const starkbank_client *client,
    const starkbank_entity *card, starkbank_entity **out, starkbank_errors **errors);
/* One card, posted to corporate-card/token. Free card with
   starkbank_entity_free; unlike POST_MULTI's list, this call borrows it. */

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* Every key in patch must carry STARKBANK_FLAG_PATCH: displayName, rules,
   tags, pin, status. */

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* sdk-python names this cancel(); see CorporateHolder's section header. */

/* ------------------------------------------------------- CorporateCardLog */
/*
 * Resource "CorporateCardLog"; endpoint "corporate-card/log", derived at run
 * time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         card RESOURCE("CorporateCard") (ro).
 * Query keys: limit, after, before, types, cardIds, ids.
 *
 * There is no corporatecard.Log pdf: sdk-python does not have one.
 */
#define STARKBANK_CORPORATE_CARD_LOG_ID       "id"
#define STARKBANK_CORPORATE_CARD_LOG_CREATED  "created"
#define STARKBANK_CORPORATE_CARD_LOG_TYPE     "type"
#define STARKBANK_CORPORATE_CARD_LOG_CARD     "card"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_card_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);


/* =========================================================================
 *                              CorporateHolder
 * =========================================================================
 *
 * Fields (wire keys; * = required on create, + = also accepted in an update).
 * Checked against the field table by tools/drift.py.
 *
 *   name*+ STRING                centerId+ STRING
 *   permissions+ LIST_RESOURCE("CorporateHolder.Permission")
 *   rules+ LIST_RESOURCE("CorporateRule")              tags+ LIST_STRING
 *   status+ STRING
 *   id STRING (ro)                                     updated created DATETIME (ro)
 *
 * Query keys: limit, after, before, ids, status, tags, expand.
 *
 * expand is listed as a query key even though no field carries it: QUERY and
 * PAGE already forward any bare key their params entity is given, so a caller
 * who lists or pages holders may set "expand" to "rules" today. create() and
 * get() cannot yet - neither verb shape takes a query of its own - which is
 * why sdk-python's expand keyword on those two is not modelled here.
 *
 * sdk-python spells the delete verb cancel(); sdk-c keeps the engine's DELETE_ID
 * naming (starkbank_corporate_holder_delete) for the same reason sdk-c4's
 * Institution keeps PAGE's own name instead of manufacturing a query() that
 * is not one - see known-drift.json's verb.new/verb.gone:CorporateHolder pair.
 */

#define STARKBANK_CORPORATE_HOLDER_NAME         "name"
#define STARKBANK_CORPORATE_HOLDER_CENTER_ID    "centerId"
#define STARKBANK_CORPORATE_HOLDER_PERMISSIONS  "permissions"
#define STARKBANK_CORPORATE_HOLDER_RULES        "rules"
#define STARKBANK_CORPORATE_HOLDER_TAGS         "tags"
#define STARKBANK_CORPORATE_HOLDER_STATUS       "status"
#define STARKBANK_CORPORATE_HOLDER_ID           "id"
#define STARKBANK_CORPORATE_HOLDER_UPDATED      "updated"
#define STARKBANK_CORPORATE_HOLDER_CREATED      "created"

/* Statuses, from the docs' enum; python's docstring gives examples only. */
#define STARKBANK_CORPORATE_HOLDER_STATUS_ACTIVE   "active"
#define STARKBANK_CORPORATE_HOLDER_STATUS_BLOCKED  "blocked"
#define STARKBANK_CORPORATE_HOLDER_STATUS_CANCELED "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_create(const starkbank_client *client,
    const starkbank_list *holders, starkbank_list **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* sdk-python names this cancel(); see the section header. */

/* -------------------------------------------------------------- Permission */
/* Access granted to a user for a particular CorporateHolder; embedded in its
 * "permissions" list, never fetched on its own. Registered under its bare
 * python name (corporateholder.__permission.py's _resource, not
 * _sub_resource), the same way Split is "Split" and not "Invoice.Split".
 * Fields: ownerId ownerType STRING, ownerEmail ownerName ownerPictureUrl
 *         ownerStatus STRING (ro), created DATETIME (ro).
 * No id: sdk-python's Permission is a SubResource, not a Resource.
 */
#define STARKBANK_PERMISSION_OWNER_ID          "ownerId"
#define STARKBANK_PERMISSION_OWNER_TYPE        "ownerType"
#define STARKBANK_PERMISSION_OWNER_EMAIL       "ownerEmail"
#define STARKBANK_PERMISSION_OWNER_NAME        "ownerName"
#define STARKBANK_PERMISSION_OWNER_PICTURE_URL "ownerPictureUrl"
#define STARKBANK_PERMISSION_OWNER_STATUS      "ownerStatus"
#define STARKBANK_PERMISSION_CREATED           "created"

STARKBANK_API int STARKBANK_CALL starkbank_permission_new(starkbank_entity **out);

/* ------------------------------------------------------ CorporateHolderLog */
/*
 * Resource "CorporateHolderLog"; endpoint "corporate-holder/log", derived at
 * run time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         holder RESOURCE("CorporateHolder") (ro).
 * Query keys: limit, after, before, types, holderIds, ids.
 *
 * There is no corporateholder.Log pdf: sdk-python does not have one.
 */
#define STARKBANK_CORPORATE_HOLDER_LOG_ID       "id"
#define STARKBANK_CORPORATE_HOLDER_LOG_CREATED  "created"
#define STARKBANK_CORPORATE_HOLDER_LOG_TYPE     "type"
#define STARKBANK_CORPORATE_HOLDER_LOG_HOLDER   "holder"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_holder_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);


/* =========================================================================
 *                            CorporatePurchase
 * =========================================================================
 *
 * Every field RO: sdk-python's module has no create() (a network authorizes
 * a purchase, nobody posts one) and no update(), even though the docs show a
 * documented PATCH /v2/corporate-purchase/:id - python is normative for the
 * verb surface, so this table carries no PATCH_ID verb.
 *
 * Fields (wire keys).
 *
 *   holderId holderName centerId cardId cardEnding description STRING (ro)
 *   amount tax issuerAmount merchantAmount merchantFee AMOUNT (ro)
 *   issuerCurrencyCode issuerCurrencySymbol merchantCurrencyCode
 *     merchantCurrencySymbol merchantCategoryCode merchantCategoryType
 *     merchantCountryCode merchantName merchantDisplayName merchantDisplayUrl
 *     methodCode status STRING (ro)
 *   tags corporateTransactionIds LIST_STRING (ro)
 *   id STRING (ro)                                    updated created DATETIME (ro).
 *
 * Query keys: ids, limit, after, before, merchantCategoryTypes, holderIds, cardIds, status.
 */

#define STARKBANK_CORPORATE_PURCHASE_HOLDER_ID                 "holderId"
#define STARKBANK_CORPORATE_PURCHASE_HOLDER_NAME               "holderName"
#define STARKBANK_CORPORATE_PURCHASE_CENTER_ID                 "centerId"
#define STARKBANK_CORPORATE_PURCHASE_CARD_ID                   "cardId"
#define STARKBANK_CORPORATE_PURCHASE_CARD_ENDING               "cardEnding"
#define STARKBANK_CORPORATE_PURCHASE_DESCRIPTION               "description"
#define STARKBANK_CORPORATE_PURCHASE_AMOUNT                    "amount"
#define STARKBANK_CORPORATE_PURCHASE_TAX                       "tax"
#define STARKBANK_CORPORATE_PURCHASE_ISSUER_AMOUNT             "issuerAmount"
#define STARKBANK_CORPORATE_PURCHASE_ISSUER_CURRENCY_CODE      "issuerCurrencyCode"
#define STARKBANK_CORPORATE_PURCHASE_ISSUER_CURRENCY_SYMBOL    "issuerCurrencySymbol"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_AMOUNT           "merchantAmount"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_CURRENCY_CODE    "merchantCurrencyCode"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_CURRENCY_SYMBOL  "merchantCurrencySymbol"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_CATEGORY_CODE    "merchantCategoryCode"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_CATEGORY_TYPE    "merchantCategoryType"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_COUNTRY_CODE     "merchantCountryCode"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_NAME             "merchantName"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_DISPLAY_NAME     "merchantDisplayName"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_DISPLAY_URL      "merchantDisplayUrl"
#define STARKBANK_CORPORATE_PURCHASE_MERCHANT_FEE              "merchantFee"
#define STARKBANK_CORPORATE_PURCHASE_METHOD_CODE               "methodCode"
#define STARKBANK_CORPORATE_PURCHASE_TAGS                      "tags"
#define STARKBANK_CORPORATE_PURCHASE_CORPORATE_TRANSACTION_IDS "corporateTransactionIds"
#define STARKBANK_CORPORATE_PURCHASE_STATUS                    "status"
#define STARKBANK_CORPORATE_PURCHASE_ID                        "id"
#define STARKBANK_CORPORATE_PURCHASE_UPDATED                   "updated"
#define STARKBANK_CORPORATE_PURCHASE_CREATED                   "created"

/* Statuses and method codes, from the docs' enums. */
#define STARKBANK_CORPORATE_PURCHASE_STATUS_APPROVED  "approved"
#define STARKBANK_CORPORATE_PURCHASE_STATUS_CANCELED  "canceled"
#define STARKBANK_CORPORATE_PURCHASE_STATUS_DENIED    "denied"
#define STARKBANK_CORPORATE_PURCHASE_STATUS_CONFIRMED "confirmed"
#define STARKBANK_CORPORATE_PURCHASE_STATUS_VOIDED    "voided"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_parse(const starkbank_client *client,
    const char *content, size_t content_len, const char *signature_base64,
    starkbank_entity **out, starkbank_errors **errors);
/* Verifies a purchase authorization request against Stark's public key, then
   hydrates the body directly as a CorporatePurchase - unlike
   starkbank_parse_and_verify, there is no envelope key to unwrap.
   STARKCORE_ERROR_SIGNATURE when it does not check out. */

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_response(const char *status,
    int has_amount, double amount, const char *reason, const char *tags, char **out);
/* Builds the JSON body a caller's own HTTP handler answers a
   starkbank_corporate_purchase_parse authorization request with; makes no
   network call. has_amount 0 and reason/tags NULL or "" all mean "omit this
   key", matching sdk-python's api_json dropping a None value rather than
   sending it as null - has_amount exists because amount 0 is a legal cents
   value sdk-python's default None must stay distinguishable from. tags is
   comma-joined, like STARKBANK_VERB_CONTENT_QUERY's hiddenFields. *out is
   malloc'd; free with starkbank_free. */

/* ---------------------------------------------------- CorporatePurchaseLog */
/*
 * Resource "CorporatePurchaseLog"; endpoint "corporate-purchase/log", derived
 * at run time.
 * Fields: id type STRING (ro), errors LIST_OBJECT (ro), description
 *         corporateTransactionId STRING (ro), purchase RESOURCE("CorporatePurchase") (ro),
 *         created DATETIME (ro).
 * Query keys: ids, limit, after, before, types, purchaseIds.
 *
 * errors is a list of {code, message} objects here, unlike CorporateCardLog
 * and CorporateHolderLog, which carry no errors field at all - the backend
 * sends purchase authorization failures as structured errors and card/holder
 * lifecycle events as none. There is no corporatepurchase.Log pdf.
 */
#define STARKBANK_CORPORATE_PURCHASE_LOG_ID                       "id"
#define STARKBANK_CORPORATE_PURCHASE_LOG_CREATED                  "created"
#define STARKBANK_CORPORATE_PURCHASE_LOG_TYPE                     "type"
#define STARKBANK_CORPORATE_PURCHASE_LOG_ERRORS                   "errors"
#define STARKBANK_CORPORATE_PURCHASE_LOG_DESCRIPTION              "description"
#define STARKBANK_CORPORATE_PURCHASE_LOG_CORPORATE_TRANSACTION_ID "corporateTransactionId"
#define STARKBANK_CORPORATE_PURCHASE_LOG_PURCHASE                 "purchase"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_log_params_new(
    starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_log_get(
    const starkbank_client *client, const char *id, starkbank_entity **out,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_log_query(
    const starkbank_client *client, const starkbank_entity *params, int limit,
    starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_log_page(
    const starkbank_client *client, const starkbank_entity *params, starkbank_list **out,
    char **out_cursor, starkbank_errors **errors);


/* =========================================================================
 *                               CorporateRule
 * =========================================================================
 *
 * A spending rule embedded in CorporateHolder.rules and CorporateCard.rules.
 * Bare top-level resource - sdk-python's _resource, not _sub_resource - with
 * no REST verbs of its own, the same shape as Split. No PATCH bit anywhere:
 * python's corporaterule module has no update(), so a caller changes a rule
 * by resending the owning holder's or card's whole "rules" list. categories,
 * countries and methods are opaque objects, not LIST_RESOURCE: their python
 * counterparts (MerchantCategory, MerchantCountry, CardMethod) are not
 * registered in this build, so a caller sends them exactly as the docs show,
 * e.g. [{"code": "fastFoodRestaurants"}].
 *
 * Fields (wire keys; * = required on create).
 *
 *   name*  STRING              amount* AMOUNT
 *   interval schedule currencyCode STRING
 *   purposes LIST_STRING
 *   categories countries methods LIST_OBJECT
 *   id currencySymbol currencyName STRING (ro)      counterAmount AMOUNT (ro).
 */

#define STARKBANK_CORPORATE_RULE_NAME            "name"
#define STARKBANK_CORPORATE_RULE_AMOUNT          "amount"
#define STARKBANK_CORPORATE_RULE_INTERVAL        "interval"
#define STARKBANK_CORPORATE_RULE_SCHEDULE        "schedule"
#define STARKBANK_CORPORATE_RULE_PURPOSES        "purposes"
#define STARKBANK_CORPORATE_RULE_CURRENCY_CODE   "currencyCode"
#define STARKBANK_CORPORATE_RULE_CATEGORIES      "categories"
#define STARKBANK_CORPORATE_RULE_COUNTRIES       "countries"
#define STARKBANK_CORPORATE_RULE_METHODS         "methods"
#define STARKBANK_CORPORATE_RULE_ID              "id"
#define STARKBANK_CORPORATE_RULE_COUNTER_AMOUNT  "counterAmount"
#define STARKBANK_CORPORATE_RULE_CURRENCY_SYMBOL "currencySymbol"
#define STARKBANK_CORPORATE_RULE_CURRENCY_NAME   "currencyName"

/* Intervals, from the docs' enum. */
#define STARKBANK_CORPORATE_RULE_INTERVAL_INSTANT  "instant"
#define STARKBANK_CORPORATE_RULE_INTERVAL_DAY      "day"
#define STARKBANK_CORPORATE_RULE_INTERVAL_WEEK     "week"
#define STARKBANK_CORPORATE_RULE_INTERVAL_MONTH    "month"
#define STARKBANK_CORPORATE_RULE_INTERVAL_YEAR     "year"
#define STARKBANK_CORPORATE_RULE_INTERVAL_LIFETIME "lifetime"

/* Purposes, from the docs' enum. */
#define STARKBANK_CORPORATE_RULE_PURPOSE_PURCHASE    "purchase"
#define STARKBANK_CORPORATE_RULE_PURPOSE_WITHDRAWAL  "withdrawal"
#define STARKBANK_CORPORATE_RULE_PURPOSE_VERIFICATION "verification"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_rule_new(starkbank_entity **out);


/* =========================================================================
 *                              CorporateBalance
 * =========================================================================
 *
 * The degenerate shape, again: one object, no id, no filters, one verb - see
 * Balance.
 *
 * Fields: id currency STRING (ro), amount limit maxLimit AMOUNT (ro), updated
 * DATETIME (ro).
 */

#define STARKBANK_CORPORATE_BALANCE_ID         "id"
#define STARKBANK_CORPORATE_BALANCE_AMOUNT     "amount"
#define STARKBANK_CORPORATE_BALANCE_LIMIT      "limit"
#define STARKBANK_CORPORATE_BALANCE_MAX_LIMIT  "maxLimit"
#define STARKBANK_CORPORATE_BALANCE_CURRENCY   "currency"
#define STARKBANK_CORPORATE_BALANCE_UPDATED    "updated"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_balance_get(const starkbank_client *client,
    starkbank_entity **out, starkbank_errors **errors);


/* =========================================================================
 *                             CorporateInvoice
 * =========================================================================
 *
 * create() posts a single object - sdk-python's rest.post_single, the same
 * shape as Webhook - so this uses STARKBANK_VERB_POST_SINGLE, not
 * POST_MULTI. There is no get(): sdk-python's corporateinvoice module
 * defines only create(), query() and page(), so this table has no GET_ID
 * verb either.
 *
 * taxId, name, brcode, due, link, status, corporateTransactionId, updated
 * and created are all sdk-python __init__ keywords, but every one of them is
 * filed under the class docstring's "Attributes (return-only)", not under
 * "## Parameters" - only amount is required and tags is optional at create -
 * so they carry RO here and not CREATE, the same table convention
 * CorporateCard.pin and CorporateWithdrawal's section below already use.
 *
 * due is DATE_OR_DATETIME, not DATETIME: sdk-python coerces it with
 * check_datetime_or_date, the same coercion Invoice.due uses.
 *
 * Fields (wire keys; * = required on create).
 *
 *   amount* AMOUNT
 *   tags LIST_STRING
 *   taxId name brcode link status corporateTransactionId id STRING (ro)
 *   due DATE_OR_DATETIME (ro)
 *   updated created DATETIME (ro).
 *
 * Query keys: limit, after, before, status, tags.
 */

#define STARKBANK_CORPORATE_INVOICE_AMOUNT                   "amount"
#define STARKBANK_CORPORATE_INVOICE_TAGS                     "tags"
#define STARKBANK_CORPORATE_INVOICE_TAX_ID                   "taxId"
#define STARKBANK_CORPORATE_INVOICE_NAME                     "name"
#define STARKBANK_CORPORATE_INVOICE_BRCODE                   "brcode"
#define STARKBANK_CORPORATE_INVOICE_DUE                      "due"
#define STARKBANK_CORPORATE_INVOICE_LINK                     "link"
#define STARKBANK_CORPORATE_INVOICE_STATUS                   "status"
#define STARKBANK_CORPORATE_INVOICE_CORPORATE_TRANSACTION_ID "corporateTransactionId"
#define STARKBANK_CORPORATE_INVOICE_ID                       "id"
#define STARKBANK_CORPORATE_INVOICE_UPDATED                  "updated"
#define STARKBANK_CORPORATE_INVOICE_CREATED                  "created"

/* Statuses, from the docs' enum. */
#define STARKBANK_CORPORATE_INVOICE_STATUS_CREATED "created"
#define STARKBANK_CORPORATE_INVOICE_STATUS_EXPIRED "expired"
#define STARKBANK_CORPORATE_INVOICE_STATUS_OVERDUE "overdue"
#define STARKBANK_CORPORATE_INVOICE_STATUS_PAID    "paid"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_invoice_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_invoice_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_invoice_create(const starkbank_client *client,
    const starkbank_entity *invoice, starkbank_entity **out, starkbank_errors **errors);
/* One invoice, posted to corporate-invoice. The entity stays yours - unlike
   POST_MULTI's list, this does not take ownership. */

STARKBANK_API int STARKBANK_CALL starkbank_corporate_invoice_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_invoice_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);


/* =========================================================================
 *                            CorporateTransaction
 * =========================================================================
 *
 * A read-only ledger entry. sdk-python's module has no create(), no
 * update() and no delete() - a caller never posts a transaction, the API
 * writes one for every balance shift (a purchase, an invoice, a withdrawal)
 * - so this table has no CREATE bit anywhere and no NEW/POST verb.
 *
 * query()'s actual signature takes source, tags, external_ids, after,
 * before, ids and limit; its docstring also prose-lists a "status" filter
 * that signature does not accept, and the docs' GET /v2/corporate-transaction
 * endpoint lists neither ids nor externalIds at all. Code is normative over
 * docstring prose and over the docs' parameter list here, so "status" stays
 * out of STARKBANK_CORPORATE_TRANSACTION_QUERY while ids and externalIds
 * stay in - see known-drift.json's query.gone:CorporateTransaction:ids and
 * query.gone:CorporateTransaction:externalIds.
 *
 * Fields (wire keys), every one read-only: a caller never posts a
 * transaction.
 *
 *   id description source STRING (ro)
 *   amount balance AMOUNT (ro)
 *   tags LIST_STRING (ro)
 *   created DATETIME (ro).
 *
 * Query keys: limit, after, before, tags, externalIds, ids, source.
 */

#define STARKBANK_CORPORATE_TRANSACTION_ID          "id"
#define STARKBANK_CORPORATE_TRANSACTION_AMOUNT      "amount"
#define STARKBANK_CORPORATE_TRANSACTION_BALANCE     "balance"
#define STARKBANK_CORPORATE_TRANSACTION_DESCRIPTION "description"
#define STARKBANK_CORPORATE_TRANSACTION_SOURCE      "source"
#define STARKBANK_CORPORATE_TRANSACTION_TAGS        "tags"
#define STARKBANK_CORPORATE_TRANSACTION_CREATED     "created"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_transaction_params_new(
    starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_transaction_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_transaction_query(
    const starkbank_client *client, const starkbank_entity *params, int limit,
    starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_transaction_page(
    const starkbank_client *client, const starkbank_entity *params, starkbank_list **out,
    char **out_cursor, starkbank_errors **errors);


/* =========================================================================
 *                            CorporateWithdrawal
 * =========================================================================
 *
 * create() posts a single object, the same shape as CorporateInvoice and
 * Webhook, so this uses STARKBANK_VERB_POST_SINGLE, not POST_MULTI.
 *
 * amount and externalId are the only two "## Parameters (required)" entries
 * in sdk-python's class docstring and the only two constructor keywords with
 * no default; tags is the lone "## Parameters (optional)" entry.
 * transactionId, corporateTransactionId, updated and created are
 * constructor keywords too but are filed under "Attributes (return-only)",
 * so they carry RO here, not CREATE - the same convention CorporateInvoice's
 * section above uses.
 *
 * The docs' GET /v2/corporate-withdrawal endpoint does not list externalId
 * as a filter; sdk-python's query()/page() accept external_ids anyway, and
 * python is normative for the verb surface - see known-drift.json's
 * query.gone:CorporateWithdrawal:externalIds.
 *
 * Fields (wire keys; * = required on create).
 *
 *   amount* AMOUNT
 *   externalId* STRING
 *   tags LIST_STRING
 *   transactionId corporateTransactionId id STRING (ro)
 *   updated created DATETIME (ro).
 *
 * Query keys: limit, after, before, tags, externalIds.
 */

#define STARKBANK_CORPORATE_WITHDRAWAL_AMOUNT                   "amount"
#define STARKBANK_CORPORATE_WITHDRAWAL_EXTERNAL_ID              "externalId"
#define STARKBANK_CORPORATE_WITHDRAWAL_TAGS                     "tags"
#define STARKBANK_CORPORATE_WITHDRAWAL_TRANSACTION_ID           "transactionId"
#define STARKBANK_CORPORATE_WITHDRAWAL_CORPORATE_TRANSACTION_ID "corporateTransactionId"
#define STARKBANK_CORPORATE_WITHDRAWAL_ID                       "id"
#define STARKBANK_CORPORATE_WITHDRAWAL_UPDATED                  "updated"
#define STARKBANK_CORPORATE_WITHDRAWAL_CREATED                  "created"

STARKBANK_API int STARKBANK_CALL starkbank_corporate_withdrawal_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_withdrawal_params_new(
    starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_withdrawal_create(
    const starkbank_client *client, const starkbank_entity *withdrawal, starkbank_entity **out,
    starkbank_errors **errors);
/* One withdrawal, posted to corporate-withdrawal. The entity stays yours -
   unlike POST_MULTI's list, this does not take ownership. */

STARKBANK_API int STARKBANK_CALL starkbank_corporate_withdrawal_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_withdrawal_query(
    const starkbank_client *client, const starkbank_entity *params, int limit,
    starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_corporate_withdrawal_page(
    const starkbank_client *client, const starkbank_entity *params, starkbank_list **out,
    char **out_cursor, starkbank_errors **errors);


/* =========================================================================
 *                               DarfPayment
 * =========================================================================
 *
 * Pay a DARF (Documento de Arrecadacao de Receitas Federais) without a bar
 * code: every field below is structured, so - unlike UtilityPayment and
 * TaxPayment - there is no conditionally-required line/barCode pair and
 * every create field that sdk-python requires carries REQUIRED outright.
 * DarfPayment has no update verb, so nothing here is patchable, and it has
 * no "type" attribute: sdk-python's DarfPayment carries none.
 *
 * Fields (wire keys; * = required on create).
 *
 *   description* revenueCode* taxId* STRING
 *   competence* DATE           nominalAmount* fineAmount* interestAmount*
 *   AMOUNT                     due* DATE
 *   referenceNumber STRING     scheduled DATE          tags LIST_STRING
 *   id status STRING (ro)      amount fee AMOUNT (ro)
 *   transactionIds LIST_STRING (ro)      updated created DATETIME (ro)
 *
 * Query keys: limit, after, before, tags, ids, status.
 *
 * competence, due and scheduled are all plain DATE: sdk-python calls
 * check_date on each and none carries Invoice.due's scheduled-invoice
 * alternate meaning.
 */
#define STARKBANK_DARF_PAYMENT_DESCRIPTION      "description"
#define STARKBANK_DARF_PAYMENT_REVENUE_CODE     "revenueCode"
#define STARKBANK_DARF_PAYMENT_TAX_ID           "taxId"
#define STARKBANK_DARF_PAYMENT_COMPETENCE       "competence"
#define STARKBANK_DARF_PAYMENT_NOMINAL_AMOUNT   "nominalAmount"
#define STARKBANK_DARF_PAYMENT_FINE_AMOUNT      "fineAmount"
#define STARKBANK_DARF_PAYMENT_INTEREST_AMOUNT  "interestAmount"
#define STARKBANK_DARF_PAYMENT_DUE              "due"
#define STARKBANK_DARF_PAYMENT_REFERENCE_NUMBER "referenceNumber"
#define STARKBANK_DARF_PAYMENT_SCHEDULED        "scheduled"
#define STARKBANK_DARF_PAYMENT_TAGS             "tags"
#define STARKBANK_DARF_PAYMENT_ID               "id"
#define STARKBANK_DARF_PAYMENT_STATUS           "status"
#define STARKBANK_DARF_PAYMENT_AMOUNT           "amount"
#define STARKBANK_DARF_PAYMENT_FEE              "fee"
#define STARKBANK_DARF_PAYMENT_TRANSACTION_IDS  "transactionIds"
#define STARKBANK_DARF_PAYMENT_UPDATED          "updated"
#define STARKBANK_DARF_PAYMENT_CREATED          "created"

#define STARKBANK_DARF_PAYMENT_STATUS_CREATED    "created"
#define STARKBANK_DARF_PAYMENT_STATUS_PROCESSING "processing"
#define STARKBANK_DARF_PAYMENT_STATUS_CONFIRMED  "confirmed"
#define STARKBANK_DARF_PAYMENT_STATUS_SUCCESS    "success"
#define STARKBANK_DARF_PAYMENT_STATUS_FAILED     "failed"
#define STARKBANK_DARF_PAYMENT_STATUS_CANCELED   "canceled"

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_create(const starkbank_client *client,
    const starkbank_list *payments, starkbank_list **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_delete(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* Only cancels a payment that has not started processing yet; a payment
   already processed can still be deleted but is not reversed. Yours to
   free. */

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);
/* Only valid for a payment with "success", "processing" or "created" status.
   Free with starkbank_free. */

/* ----------------------------------------------------------- DarfPaymentLog */
/*
 * Resource "DarfPaymentLog"; endpoint "darf-payment/log", derived at run
 * time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), payment RESOURCE("DarfPayment") (ro).
 * Query keys: limit, after, before, types, paymentIds.
 *
 * There is no darfpayment.Log pdf: sdk-python does not have one, and python
 * is normative for the verb surface. The receipt is
 * starkbank_darf_payment_pdf.
 */
#define STARKBANK_DARF_PAYMENT_LOG_ID       "id"
#define STARKBANK_DARF_PAYMENT_LOG_CREATED  "created"
#define STARKBANK_DARF_PAYMENT_LOG_TYPE     "type"
#define STARKBANK_DARF_PAYMENT_LOG_ERRORS   "errors"
#define STARKBANK_DARF_PAYMENT_LOG_PAYMENT  "payment"

STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_darf_payment_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                 Deposit
 * =========================================================================
 *
 * Deposit models passive cash-in received from an external transfer.
 * sdk-python's Deposit has no create() at all - every field is return-only -
 * and the only mutation the API exposes is reversal: rest.patch_id with a
 * single "amount" key, full or partial, where amount=0 fully reverses the
 * deposit. There is therefore no NEW and no create verb here, only
 * PARAMS (the query and patch bag), GET_ID, QUERY, PAGE and PATCH_ID.
 *
 * Fields (wire keys; + = patchable). Every field but amount is return-only.
 *
 *   amount+ AMOUNT
 *   id name taxId bankCode branchCode accountNumber accountType type status STRING (ro)
 *   fee AMOUNT (ro)
 *   tags transactionIds LIST_STRING (ro)
 *   created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, status, sort, tags, ids.
 *
 * amount carries PATCH and neither CREATE nor REQUIRED: REQUIRED means
 * "required on create" and this resource has no create to be required for.
 * sdk-python's own update(id, amount=None, user=None) defaults amount to
 * None despite the docstring's prose calling it required - the same
 * update()-is-not-create asymmetry Invoice.status and BrcodePayment.status
 * already carry. A caller who omits it gets the API's own error.
 */
#define STARKBANK_DEPOSIT_ID                "id"
#define STARKBANK_DEPOSIT_NAME              "name"
#define STARKBANK_DEPOSIT_TAX_ID            "taxId"
#define STARKBANK_DEPOSIT_BANK_CODE         "bankCode"
#define STARKBANK_DEPOSIT_BRANCH_CODE       "branchCode"
#define STARKBANK_DEPOSIT_ACCOUNT_NUMBER    "accountNumber"
#define STARKBANK_DEPOSIT_ACCOUNT_TYPE      "accountType"
#define STARKBANK_DEPOSIT_AMOUNT            "amount"
#define STARKBANK_DEPOSIT_TYPE              "type"
#define STARKBANK_DEPOSIT_STATUS            "status"
#define STARKBANK_DEPOSIT_TAGS              "tags"
#define STARKBANK_DEPOSIT_FEE               "fee"
#define STARKBANK_DEPOSIT_TRANSACTION_IDS   "transactionIds"
#define STARKBANK_DEPOSIT_CREATED           "created"
#define STARKBANK_DEPOSIT_UPDATED           "updated"

#define STARKBANK_DEPOSIT_STATUS_CREATED   "created"
#define STARKBANK_DEPOSIT_STATUS_VOID      "void"

STARKBANK_API int STARKBANK_CALL starkbank_deposit_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_deposit_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_deposit_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_deposit_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_deposit_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* amount is the only patchable key: pass 0 to fully reverse, or a smaller
   amount to partially reverse. */

/* ------------------------------------------------------------- deposit.Log */
/*
 * Resource "DepositLog"; endpoint "deposit/log", derived at run time.
 * Fields: id STRING (ro), created DATETIME (ro), type STRING (ro),
 *         errors LIST_STRING (ro), deposit RESOURCE("Deposit") (ro).
 * Query keys: limit, after, before, types, depositIds.
 *
 * Unlike transfer.Log, boleto.Log, boletopayment.Log and brcodepayment.Log,
 * deposit.Log has a pdf verb: the reversed deposit's receipt.
 */
#define STARKBANK_DEPOSIT_LOG_ID        "id"
#define STARKBANK_DEPOSIT_LOG_CREATED   "created"
#define STARKBANK_DEPOSIT_LOG_TYPE      "type"
#define STARKBANK_DEPOSIT_LOG_ERRORS    "errors"
#define STARKBANK_DEPOSIT_LOG_DEPOSIT   "deposit"

STARKBANK_API int STARKBANK_CALL starkbank_deposit_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_deposit_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_deposit_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_deposit_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_deposit_log_pdf(const starkbank_client *client,
    const char *id, unsigned char **out, size_t *out_len, starkbank_errors **errors);

/* =========================================================================
 *                                 DictKey
 * =========================================================================
 *
 * Query/get only: sdk-python's DictKey has no create() (an EVP key is
 * created automatically for every new Workspace) and no update(); every
 * attribute is return-only. id is the PIX key itself (an email, a tax id, a
 * phone number, or a DICT-issued EVP uuid).
 *
 * Fields (all return-only): id type name taxId ownerType bankName ispb
 *   branchCode accountNumber accountType status STRING (ro).
 *
 * Query keys: limit, type, after, before, ids, status.
 */
#define STARKBANK_DICT_KEY_ID               "id"
#define STARKBANK_DICT_KEY_TYPE             "type"
#define STARKBANK_DICT_KEY_NAME             "name"
#define STARKBANK_DICT_KEY_TAX_ID           "taxId"
#define STARKBANK_DICT_KEY_OWNER_TYPE       "ownerType"
#define STARKBANK_DICT_KEY_BANK_NAME        "bankName"
#define STARKBANK_DICT_KEY_ISPB             "ispb"
#define STARKBANK_DICT_KEY_BRANCH_CODE      "branchCode"
#define STARKBANK_DICT_KEY_ACCOUNT_NUMBER   "accountNumber"
#define STARKBANK_DICT_KEY_ACCOUNT_TYPE     "accountType"
#define STARKBANK_DICT_KEY_STATUS           "status"

#define STARKBANK_DICT_KEY_TYPE_CPF    "cpf"
#define STARKBANK_DICT_KEY_TYPE_CNPJ   "cnpj"
#define STARKBANK_DICT_KEY_TYPE_PHONE  "phone"
#define STARKBANK_DICT_KEY_TYPE_EMAIL  "email"
#define STARKBANK_DICT_KEY_TYPE_EVP    "evp"

#define STARKBANK_DICT_KEY_STATUS_CREATED     "created"
#define STARKBANK_DICT_KEY_STATUS_REGISTERED  "registered"
#define STARKBANK_DICT_KEY_STATUS_CANCELED    "canceled"
#define STARKBANK_DICT_KEY_STATUS_FAILED      "failed"

STARKBANK_API int STARKBANK_CALL starkbank_dict_key_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_dict_key_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
/* id is the PIX key. This looks up keys you do not own, so it can validate a
   key before a Transfer to it - but avoid looking up keys without following
   up with a transfer: Bacen blocks accounts that make too many standalone
   lookups in a short time, invalid-key lookups included. The returned
   encrypted branchCode/accountNumber can be passed straight into a Transfer
   without decrypting them. */

STARKBANK_API int STARKBANK_CALL starkbank_dict_key_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_dict_key_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                Institution
 * =========================================================================
 *
 * sdk-python models Institution as a SubResource, not a Resource: there is no
 * id field, and the module's only function is query(), which is itself
 * `rest.get_page(...)[0]` - one page call, cursor thrown away, no looping.
 * That is the PAGE shape wearing a different name, so sdk-c exposes
 * starkbank_institution_page directly rather than inventing an iterator
 * python's own code never builds. Pass NULL for out_cursor to match python's
 * query() exactly.
 *
 * Fields (all return-only): displayName name spiCode strCode STRING (ro).
 * Query keys: limit, search, spiCodes, strCodes.
 */
#define STARKBANK_INSTITUTION_DISPLAY_NAME  "displayName"
#define STARKBANK_INSTITUTION_NAME          "name"
#define STARKBANK_INSTITUTION_SPI_CODE      "spiCode"
#define STARKBANK_INSTITUTION_STR_CODE      "strCode"

STARKBANK_API int STARKBANK_CALL starkbank_institution_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_institution_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                Transaction
 * =========================================================================
 *
 * sdk-python's create() is deprecated since v2.31.0 and unconditionally
 * raises StarkError("deprecated") - it is not a working create, it is a
 * tombstone. Transactions now arise only as a side effect of other
 * operations (a Transfer, a paid charge...), so this table has no NEW and no
 * create verb: adding one would let a C caller do, over the wire, exactly
 * what sdk-python refuses to do at all.
 *
 * Fields (all return-only): amount fee balance AMOUNT (ro)
 *   description externalId receiverId senderId source id STRING (ro)
 *   tags LIST_STRING (ro)
 *   created DATETIME (ro)
 *
 * Query keys: limit, after, before, tags, externalIds, ids.
 */
#define STARKBANK_TRANSACTION_AMOUNT        "amount"
#define STARKBANK_TRANSACTION_DESCRIPTION   "description"
#define STARKBANK_TRANSACTION_EXTERNAL_ID   "externalId"
#define STARKBANK_TRANSACTION_RECEIVER_ID   "receiverId"
#define STARKBANK_TRANSACTION_SENDER_ID     "senderId"
#define STARKBANK_TRANSACTION_TAGS          "tags"
#define STARKBANK_TRANSACTION_ID            "id"
#define STARKBANK_TRANSACTION_FEE           "fee"
#define STARKBANK_TRANSACTION_CREATED       "created"
#define STARKBANK_TRANSACTION_SOURCE        "source"
#define STARKBANK_TRANSACTION_BALANCE       "balance"

STARKBANK_API int STARKBANK_CALL starkbank_transaction_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_transaction_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_transaction_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_transaction_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                Workspace
 * =========================================================================
 *
 * create is post_single, like Webhook: sdk-python builds one Workspace
 * locally from username/name/allowedTaxIds and posts it as the body itself,
 * not wrapped in a list.
 *
 * Fields (wire keys; * = required on create, + = also patchable).
 *
 *   username*+ name*+ STRING      allowedTaxIds+ LIST_STRING
 *   id organizationId pictureUrl STRING (ro)   created DATETIME (ro)
 *   status+ picture+ STRING
 *
 * Query keys: limit, username, ids.
 *
 * picture carries PATCH alone, neither RO nor CREATE nor REQUIRED: it never
 * appears on a returned Workspace (sdk-python's Workspace class has no
 * picture attribute at all - only update() takes one), so it is the one
 * field in this table that is genuinely write-only despite not being marked
 * (ro) in the block above; the "* = required, + = patchable" legend has no
 * symbol for that shape and this note is it.
 *
 * picture is the one field with judgement in it. sdk-python's update() takes
 * picture as raw bytes and pictureType as a separate parameter, then builds
 * exactly one wire value itself:
 *     payload["picture"] = "data:{picture_type};base64,{base64(picture)}"
 * pictureType never reaches the wire as its own key - it is only ever
 * encoded into the "picture" string - so there is exactly one wire field to
 * table, and its wire representation is already a plain string. No new
 * engine verb shape is needed: the existing generic starkbank_entity_set_string
 * covers it completely. A caller sends a picture by base64-encoding the bytes
 * themselves and calling
 *     starkbank_entity_set_string(patch, STARKBANK_WORKSPACE_PICTURE,
 *         "data:image/png;base64,...")
 * - a plain const char *, the smallest possible shape, the same preference
 * STARKBANK_VERB_CONTENT_QUERY's own comment argues for boleto.pdf's two
 * string keys.
 */
#define STARKBANK_WORKSPACE_USERNAME          "username"
#define STARKBANK_WORKSPACE_NAME              "name"
#define STARKBANK_WORKSPACE_ALLOWED_TAX_IDS   "allowedTaxIds"
#define STARKBANK_WORKSPACE_ID                "id"
#define STARKBANK_WORKSPACE_STATUS            "status"
#define STARKBANK_WORKSPACE_ORGANIZATION_ID   "organizationId"
#define STARKBANK_WORKSPACE_PICTURE_URL       "pictureUrl"
#define STARKBANK_WORKSPACE_CREATED           "created"
#define STARKBANK_WORKSPACE_PICTURE           "picture"

#define STARKBANK_WORKSPACE_STATUS_ACTIVE  "active"
#define STARKBANK_WORKSPACE_STATUS_CLOSED  "closed"
#define STARKBANK_WORKSPACE_STATUS_FROZEN  "frozen"
#define STARKBANK_WORKSPACE_STATUS_BLOCKED "blocked"

STARKBANK_API int STARKBANK_CALL starkbank_workspace_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_workspace_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_workspace_create(const starkbank_client *client,
    const starkbank_entity *workspace, starkbank_entity **out, starkbank_errors **errors);
/* post_single: workspace is the body itself, not wrapped in a list - same
   shape as starkbank_webhook_create. */

STARKBANK_API int STARKBANK_CALL starkbank_workspace_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_workspace_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
/* If no filters are set and the user is an Organization, every Workspace the
   Organization owns is returned - unchanged from sdk-python. */

STARKBANK_API int STARKBANK_CALL starkbank_workspace_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_workspace_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* username, name, allowedTaxIds, status and picture are each independently
   patchable; send only the keys that are changing.
   Unlike every other patchable resource in this SDK, patching username or
   name also sends it as a URL query parameter (?username=...&name=...) in
   addition to the JSON body - sdk-python's workspace.update does the same,
   and this mirrors it wire-for-wire. */


/* =========================================================================
 *                              MerchantSession
 * =========================================================================
 *
 * purchase(uuid, purchase) needed one small addition to the engine:
 * STARKBANK_VERB_POST_SUB_RESOURCE, a POST counterpart to
 * STARKBANK_VERB_SUB_RESOURCE (Invoice.payment's GET) built on core-c's own
 * starkcore_rest_post_sub_resource, which was already exposed and unused by
 * this SDK. See starkbankVerbCreateSubResource's comment in starkc/verb.c
 * for the full reasoning and why neither existing sub-path shape fit.
 *
 * Fields (wire keys; * = required on create). MerchantSession has no
 * update(), so there is no PATCH bit anywhere in this table.
 *
 *   allowedFundingTypes* LIST_STRING
 *   allowedInstallments* LIST_RESOURCE("MerchantSession.AllowedInstallment")
 *   expiration* NUMBER
 *   allowedIps LIST_STRING          challengeMode STRING
 *   tags LIST_STRING
 *   id uuid holderId softDescriptor status STRING (ro)
 *   created updated DATETIME (ro)
 *
 * Query keys: limit, status, tags, ids, after, before, holderId.
 *
 * holderId rides in the query key list even though the docs' GET
 * /v2/merchant-session parameter list omits it: sdk-python's query()/page()
 * both take holder_id, and python is normative for the verb surface - see
 * tests/reference/known-drift.json's query.gone:MerchantSession:holderId.
 *
 * expiration is NUMBER, not SECONDS: sdk-python's __init__ does
 * "self.expiration = expiration" with no check_timedelta call, unlike
 * Invoice.expiration, so tools/drift.py's type.conflict check (normative on
 * python's actual coercion, not its "integer or datetime.timedelta"
 * docstring prose) is a hard stop here - the table follows python's real
 * behaviour rather than the naive per-field mirror.
 */

#define STARKBANK_MERCHANT_SESSION_ALLOWED_FUNDING_TYPES  "allowedFundingTypes"
#define STARKBANK_MERCHANT_SESSION_ALLOWED_INSTALLMENTS   "allowedInstallments"
#define STARKBANK_MERCHANT_SESSION_EXPIRATION             "expiration"
#define STARKBANK_MERCHANT_SESSION_ALLOWED_IPS            "allowedIps"
#define STARKBANK_MERCHANT_SESSION_CHALLENGE_MODE         "challengeMode"
#define STARKBANK_MERCHANT_SESSION_TAGS                   "tags"
#define STARKBANK_MERCHANT_SESSION_ID                     "id"
#define STARKBANK_MERCHANT_SESSION_UUID                   "uuid"
#define STARKBANK_MERCHANT_SESSION_HOLDER_ID              "holderId"
#define STARKBANK_MERCHANT_SESSION_SOFT_DESCRIPTOR        "softDescriptor"
#define STARKBANK_MERCHANT_SESSION_STATUS                 "status"
#define STARKBANK_MERCHANT_SESSION_CREATED                "created"
#define STARKBANK_MERCHANT_SESSION_UPDATED                "updated"

/* Funding types and challenge modes, from the docs' enums. */
#define STARKBANK_MERCHANT_SESSION_FUNDING_TYPE_CREDIT     "credit"
#define STARKBANK_MERCHANT_SESSION_FUNDING_TYPE_DEBIT      "debit"
#define STARKBANK_MERCHANT_SESSION_CHALLENGE_MODE_ENABLED  "enabled"
#define STARKBANK_MERCHANT_SESSION_CHALLENGE_MODE_DISABLED "disabled"

/* Statuses: the docs list "active"/"expired"/"success"; sdk-python's own
   docstring examples add "created" - both are kept since either is a real
   observed value and these are plain #define conveniences, not a validated
   enum (see include/starkbank.h's own note on why enums stay strings). */
#define STARKBANK_MERCHANT_SESSION_STATUS_CREATED  "created"
#define STARKBANK_MERCHANT_SESSION_STATUS_ACTIVE   "active"
#define STARKBANK_MERCHANT_SESSION_STATUS_EXPIRED  "expired"
#define STARKBANK_MERCHANT_SESSION_STATUS_SUCCESS  "success"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_create(const starkbank_client *client,
    const starkbank_entity *session, starkbank_entity **out, starkbank_errors **errors);
/* post_single: session is the body itself, not wrapped in a list - the same
   shape as starkbank_webhook_create. */

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_purchase(const starkbank_client *client,
    const char *uuid, const starkbank_entity *purchase, starkbank_entity **out,
    starkbank_errors **errors);
/* rest.post_sub_resource: POST purchase to merchant-session/<uuid>/purchase.
   purchase must be tagged "Purchase" (starkbank_purchase_new); the entity
   that comes back is tagged "Purchase" too and carries an id. Borrows
   purchase; free *out with starkbank_entity_free. */

/* ---------------------------------------- MerchantSession.AllowedInstallment */
/* SubResource (no id); embedded in MerchantSession.allowedInstallments.
 * Registered qualified, not bare "AllowedInstallment": sdk-python's own
 * module assigns _sub_resource, not _resource, and tools/drift.py's python
 * reader qualifies every _sub_resource by its owning package's resource name
 * regardless of whether the bare name collides with anything else - see
 * merchantsession.h.
 * Fields: totalAmount* AMOUNT, count* NUMBER.
 */
#define STARKBANK_ALLOWED_INSTALLMENT_TOTAL_AMOUNT  "totalAmount"
#define STARKBANK_ALLOWED_INSTALLMENT_COUNT         "count"

STARKBANK_API int STARKBANK_CALL starkbank_allowed_installment_new(starkbank_entity **out);

/* ---------------------------------------------------------------- Purchase */
/* Resource (has an id), reached only through
 * starkbank_merchant_session_purchase - sdk-python's Purchase module exports
 * no get/query/page of its own.
 * Fields: amount* AMOUNT; cardExpiration* cardNumber* cardSecurityCode*
 *         holderName* fundingType* STRING; holderEmail holderPhone holderId
 *         STRING; installmentCount NUMBER; billingCountryCode billingCity
 *         billingStateCode billingStreetLine1 billingStreetLine2
 *         billingZipCode STRING; metadata OBJECT; softDescriptor STRING;
 *         tags LIST_STRING; id cardEnding cardId challengeMode challengeUrl
 *         currencyCode endToEndId network source status STRING (ro);
 *         fee AMOUNT (ro); created updated DATETIME (ro).
 */
#define STARKBANK_PURCHASE_AMOUNT                "amount"
#define STARKBANK_PURCHASE_CARD_EXPIRATION       "cardExpiration"
#define STARKBANK_PURCHASE_CARD_NUMBER           "cardNumber"
#define STARKBANK_PURCHASE_CARD_SECURITY_CODE    "cardSecurityCode"
#define STARKBANK_PURCHASE_HOLDER_NAME           "holderName"
#define STARKBANK_PURCHASE_FUNDING_TYPE          "fundingType"
#define STARKBANK_PURCHASE_HOLDER_EMAIL          "holderEmail"
#define STARKBANK_PURCHASE_HOLDER_PHONE          "holderPhone"
#define STARKBANK_PURCHASE_HOLDER_ID             "holderId"
#define STARKBANK_PURCHASE_INSTALLMENT_COUNT     "installmentCount"
#define STARKBANK_PURCHASE_BILLING_COUNTRY_CODE  "billingCountryCode"
#define STARKBANK_PURCHASE_BILLING_CITY          "billingCity"
#define STARKBANK_PURCHASE_BILLING_STATE_CODE    "billingStateCode"
#define STARKBANK_PURCHASE_BILLING_STREET_LINE_1 "billingStreetLine1"
#define STARKBANK_PURCHASE_BILLING_STREET_LINE_2 "billingStreetLine2"
#define STARKBANK_PURCHASE_BILLING_ZIP_CODE      "billingZipCode"
#define STARKBANK_PURCHASE_METADATA              "metadata"
#define STARKBANK_PURCHASE_SOFT_DESCRIPTOR       "softDescriptor"
#define STARKBANK_PURCHASE_TAGS                  "tags"
#define STARKBANK_PURCHASE_ID                    "id"
#define STARKBANK_PURCHASE_CARD_ENDING           "cardEnding"
#define STARKBANK_PURCHASE_CARD_ID               "cardId"
#define STARKBANK_PURCHASE_CHALLENGE_MODE        "challengeMode"
#define STARKBANK_PURCHASE_CHALLENGE_URL         "challengeUrl"
#define STARKBANK_PURCHASE_CURRENCY_CODE         "currencyCode"
#define STARKBANK_PURCHASE_END_TO_END_ID         "endToEndId"
#define STARKBANK_PURCHASE_FEE                   "fee"
#define STARKBANK_PURCHASE_NETWORK               "network"
#define STARKBANK_PURCHASE_SOURCE                "source"
#define STARKBANK_PURCHASE_STATUS                "status"
#define STARKBANK_PURCHASE_CREATED               "created"
#define STARKBANK_PURCHASE_UPDATED               "updated"

STARKBANK_API int STARKBANK_CALL starkbank_purchase_new(starkbank_entity **out);
/* Free it, or hand it to starkbank_merchant_session_purchase, which borrows
   it and does not take ownership. */

/* ------------------------------------------------------- MerchantSessionLog */
/*
 * Resource "MerchantSessionLog"; endpoint "merchant-session/log", derived at
 * run time.
 * Fields: id type STRING (ro), errors LIST_STRING (ro),
 *         session RESOURCE("MerchantSession") (ro), created DATETIME (ro).
 * Query keys: limit, after, before, types, sessionIds.
 *
 * errors is LIST_STRING here, unlike MerchantCardLog/MerchantInstallmentLog/
 * MerchantPurchaseLog's LIST_OBJECT {code, message} pairs: this log is served
 * by the acquirer's session/challenge service, a different backing service
 * from the card ledger the other three share.
 */
#define STARKBANK_MERCHANT_SESSION_LOG_ID       "id"
#define STARKBANK_MERCHANT_SESSION_LOG_CREATED  "created"
#define STARKBANK_MERCHANT_SESSION_LOG_TYPE     "type"
#define STARKBANK_MERCHANT_SESSION_LOG_ERRORS   "errors"
#define STARKBANK_MERCHANT_SESSION_LOG_SESSION  "session"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_session_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                                MerchantCard
 * =========================================================================
 *
 * Every field is RO: sdk-python's module has no create(); a card is stored
 * once a MerchantSession Purchase or MerchantPurchase succeeds, never posted
 * directly. get/query/page are the whole verb surface.
 *
 * Fields (wire keys; every field is return-only).
 *
 *   id ending fundingType holderName network status STRING (ro)
 *   tags LIST_STRING (ro)
 *   expiration DATE_OR_DATETIME (ro)
 *   created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, status, tags, ids.
 */
#define STARKBANK_MERCHANT_CARD_ID            "id"
#define STARKBANK_MERCHANT_CARD_ENDING        "ending"
#define STARKBANK_MERCHANT_CARD_FUNDING_TYPE  "fundingType"
#define STARKBANK_MERCHANT_CARD_HOLDER_NAME   "holderName"
#define STARKBANK_MERCHANT_CARD_NETWORK       "network"
#define STARKBANK_MERCHANT_CARD_STATUS        "status"
#define STARKBANK_MERCHANT_CARD_TAGS          "tags"
#define STARKBANK_MERCHANT_CARD_EXPIRATION    "expiration"
#define STARKBANK_MERCHANT_CARD_CREATED       "created"
#define STARKBANK_MERCHANT_CARD_UPDATED       "updated"

/* Statuses, from sdk-python's docstring examples. */
#define STARKBANK_MERCHANT_CARD_STATUS_ACTIVE    "active"
#define STARKBANK_MERCHANT_CARD_STATUS_EXPIRED   "expired"
#define STARKBANK_MERCHANT_CARD_STATUS_CANCELED  "canceled"
#define STARKBANK_MERCHANT_CARD_STATUS_BLOCKED   "blocked"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* --------------------------------------------------------- MerchantCardLog */
/*
 * Resource "MerchantCardLog"; endpoint "merchant-card/log", derived at run
 * time.
 * Fields: id type STRING (ro), errors LIST_OBJECT (ro),
 *         card RESOURCE("MerchantCard") (ro),
 *         created updated DATETIME (ro).
 * Query keys: limit, cardIds, after, before, types.
 *
 * errors is a real LIST_OBJECT of {code, message} pairs, the same shape
 * CorporatePurchase.Log already established, unlike MerchantSessionLog's
 * plain LIST_STRING - see MerchantSession's section above.
 */
#define STARKBANK_MERCHANT_CARD_LOG_ID       "id"
#define STARKBANK_MERCHANT_CARD_LOG_CREATED  "created"
#define STARKBANK_MERCHANT_CARD_LOG_UPDATED  "updated"
#define STARKBANK_MERCHANT_CARD_LOG_TYPE     "type"
#define STARKBANK_MERCHANT_CARD_LOG_ERRORS   "errors"
#define STARKBANK_MERCHANT_CARD_LOG_CARD     "card"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_log_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_log_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_log_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_card_log_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* =========================================================================
 *                             MerchantInstallment
 * =========================================================================
 *
 * Every field is RO: generated automatically when a MerchantPurchase is
 * split, never posted. get/query/page are the whole verb surface.
 *
 * Fields (wire keys; every field is return-only).
 *
 *   id fundingType network purchaseId status STRING (ro)
 *   amount fee AMOUNT (ro)
 *   due DATE_OR_DATETIME (ro)
 *   tags transactionIds LIST_STRING (ro)
 *   created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, status, tags, ids, purchaseIds.
 *
 * purchaseIds is real and documented on query()/page() but the docs' GET
 * /v2/merchant-installment parameter list omits it - see
 * tests/reference/known-drift.json's
 * query.gone:MerchantInstallment:purchaseIds.
 */
#define STARKBANK_MERCHANT_INSTALLMENT_ID              "id"
#define STARKBANK_MERCHANT_INSTALLMENT_AMOUNT          "amount"
#define STARKBANK_MERCHANT_INSTALLMENT_DUE             "due"
#define STARKBANK_MERCHANT_INSTALLMENT_FEE             "fee"
#define STARKBANK_MERCHANT_INSTALLMENT_FUNDING_TYPE    "fundingType"
#define STARKBANK_MERCHANT_INSTALLMENT_NETWORK         "network"
#define STARKBANK_MERCHANT_INSTALLMENT_PURCHASE_ID     "purchaseId"
#define STARKBANK_MERCHANT_INSTALLMENT_STATUS          "status"
#define STARKBANK_MERCHANT_INSTALLMENT_TAGS            "tags"
#define STARKBANK_MERCHANT_INSTALLMENT_TRANSACTION_IDS "transactionIds"
#define STARKBANK_MERCHANT_INSTALLMENT_CREATED         "created"
#define STARKBANK_MERCHANT_INSTALLMENT_UPDATED         "updated"

/* Statuses, from sdk-python's docstring examples. */
#define STARKBANK_MERCHANT_INSTALLMENT_STATUS_CREATED  "created"
#define STARKBANK_MERCHANT_INSTALLMENT_STATUS_SUCCESS  "success"
#define STARKBANK_MERCHANT_INSTALLMENT_STATUS_FAILED   "failed"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_params_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

/* ------------------------------------------------------ MerchantInstallmentLog */
/*
 * Resource "MerchantInstallmentLog"; endpoint "merchant-installment/log",
 * derived at run time.
 * Fields: id type STRING (ro), errors LIST_OBJECT (ro),
 *         installment RESOURCE("MerchantInstallment") (ro),
 *         created updated DATETIME (ro).
 * Query keys: limit, after, before, types, installmentIds.
 */
#define STARKBANK_MERCHANT_INSTALLMENT_LOG_ID          "id"
#define STARKBANK_MERCHANT_INSTALLMENT_LOG_CREATED     "created"
#define STARKBANK_MERCHANT_INSTALLMENT_LOG_UPDATED     "updated"
#define STARKBANK_MERCHANT_INSTALLMENT_LOG_TYPE        "type"
#define STARKBANK_MERCHANT_INSTALLMENT_LOG_ERRORS      "errors"
#define STARKBANK_MERCHANT_INSTALLMENT_LOG_INSTALLMENT "installment"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_log_params_new(
    starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_log_get(
    const starkbank_client *client, const char *id, starkbank_entity **out,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_log_query(
    const starkbank_client *client, const starkbank_entity *params, int limit,
    starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_installment_log_page(
    const starkbank_client *client, const starkbank_entity *params, starkbank_list **out,
    char **out_cursor, starkbank_errors **errors);

/* =========================================================================
 *                              MerchantPurchase
 * =========================================================================
 *
 * create() is post_single, the same shape as Webhook and Workspace.
 * update(id, status, amount) sends exactly {"status": status, "amount":
 * amount} - see sdk-python's docstring: "canceled" with amount=0 cancels an
 * approved purchase, "reversed" with a lower amount partially or fully
 * reverses a confirmed one.
 *
 * Fields (wire keys; * = required on create, + = also accepted in an update).
 *
 *   amount*+ AMOUNT
 *   cardId* fundingType* STRING
 *   installmentCount* NUMBER
 *   cardExpiration cardNumber cardSecurityCode holderName holderEmail
 *     holderPhone holderId billingCountryCode billingCity billingStateCode
 *     billingStreetLine1 billingStreetLine2 billingZipCode softDescriptor
 *     STRING
 *   metadata OBJECT
 *   tags LIST_STRING
 *   id cardEnding challengeMode challengeUrl currencyCode endToEndId network
 *     source STRING (ro)
 *   fee AMOUNT (ro)
 *   status+ STRING
 *   created updated DATETIME (ro)
 *
 * Query keys: limit, after, before, status, tags, ids, holderId.
 *
 * holderId is real and documented on query()/page() but the docs' GET
 * /v2/merchant-purchase parameter list omits it - see
 * tests/reference/known-drift.json's query.gone:MerchantPurchase:holderId.
 */
#define STARKBANK_MERCHANT_PURCHASE_AMOUNT                "amount"
#define STARKBANK_MERCHANT_PURCHASE_CARD_ID               "cardId"
#define STARKBANK_MERCHANT_PURCHASE_FUNDING_TYPE          "fundingType"
#define STARKBANK_MERCHANT_PURCHASE_INSTALLMENT_COUNT     "installmentCount"
#define STARKBANK_MERCHANT_PURCHASE_CARD_EXPIRATION       "cardExpiration"
#define STARKBANK_MERCHANT_PURCHASE_CARD_NUMBER           "cardNumber"
#define STARKBANK_MERCHANT_PURCHASE_CARD_SECURITY_CODE    "cardSecurityCode"
#define STARKBANK_MERCHANT_PURCHASE_HOLDER_NAME           "holderName"
#define STARKBANK_MERCHANT_PURCHASE_HOLDER_EMAIL          "holderEmail"
#define STARKBANK_MERCHANT_PURCHASE_HOLDER_PHONE          "holderPhone"
#define STARKBANK_MERCHANT_PURCHASE_HOLDER_ID             "holderId"
#define STARKBANK_MERCHANT_PURCHASE_BILLING_COUNTRY_CODE  "billingCountryCode"
#define STARKBANK_MERCHANT_PURCHASE_BILLING_CITY          "billingCity"
#define STARKBANK_MERCHANT_PURCHASE_BILLING_STATE_CODE    "billingStateCode"
#define STARKBANK_MERCHANT_PURCHASE_BILLING_STREET_LINE_1 "billingStreetLine1"
#define STARKBANK_MERCHANT_PURCHASE_BILLING_STREET_LINE_2 "billingStreetLine2"
#define STARKBANK_MERCHANT_PURCHASE_BILLING_ZIP_CODE      "billingZipCode"
#define STARKBANK_MERCHANT_PURCHASE_METADATA              "metadata"
#define STARKBANK_MERCHANT_PURCHASE_SOFT_DESCRIPTOR       "softDescriptor"
#define STARKBANK_MERCHANT_PURCHASE_TAGS                  "tags"
#define STARKBANK_MERCHANT_PURCHASE_ID                    "id"
#define STARKBANK_MERCHANT_PURCHASE_CARD_ENDING           "cardEnding"
#define STARKBANK_MERCHANT_PURCHASE_CHALLENGE_MODE        "challengeMode"
#define STARKBANK_MERCHANT_PURCHASE_CHALLENGE_URL         "challengeUrl"
#define STARKBANK_MERCHANT_PURCHASE_CURRENCY_CODE         "currencyCode"
#define STARKBANK_MERCHANT_PURCHASE_END_TO_END_ID         "endToEndId"
#define STARKBANK_MERCHANT_PURCHASE_FEE                   "fee"
#define STARKBANK_MERCHANT_PURCHASE_NETWORK               "network"
#define STARKBANK_MERCHANT_PURCHASE_SOURCE                "source"
#define STARKBANK_MERCHANT_PURCHASE_STATUS                "status"
#define STARKBANK_MERCHANT_PURCHASE_CREATED               "created"
#define STARKBANK_MERCHANT_PURCHASE_UPDATED               "updated"

/* Statuses, from sdk-python's docstrings (class + update()). */
#define STARKBANK_MERCHANT_PURCHASE_STATUS_APPROVED   "approved"
#define STARKBANK_MERCHANT_PURCHASE_STATUS_CONFIRMED  "confirmed"
#define STARKBANK_MERCHANT_PURCHASE_STATUS_CANCELED   "canceled"
#define STARKBANK_MERCHANT_PURCHASE_STATUS_REVERSED   "reversed"
#define STARKBANK_MERCHANT_PURCHASE_STATUS_VOIDED     "voided"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_new(starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_params_new(starkbank_entity **out);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_create(const starkbank_client *client,
    const starkbank_entity *purchase, starkbank_entity **out, starkbank_errors **errors);
/* post_single: purchase is the body itself, not wrapped in a list. */

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_get(const starkbank_client *client,
    const char *id, starkbank_entity **out, starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_query(const starkbank_client *client,
    const starkbank_entity *params, int limit, starkbank_iter **out);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_page(const starkbank_client *client,
    const starkbank_entity *params, starkbank_list **out, char **out_cursor,
    starkbank_errors **errors);

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_update(const starkbank_client *client,
    const char *id, const starkbank_entity *patch, starkbank_entity **out,
    starkbank_errors **errors);
/* Every key in patch must carry STARKBANK_FLAG_PATCH: status, amount. */

/* ----------------------------------------------------- MerchantPurchaseLog */
/*
 * Resource "MerchantPurchaseLog"; endpoint "merchant-purchase/log", derived
 * at run time.
 * Fields: id type STRING (ro), errors LIST_OBJECT (ro),
 *         purchase RESOURCE("MerchantPurchase") (ro), created DATETIME (ro).
 * Query keys: limit, after, before, types, purchaseIds.
 *
 * No updated field: unlike MerchantCardLog and MerchantInstallmentLog,
 * sdk-python's merchantpurchase.Log.__init__ has no updated parameter.
 */
#define STARKBANK_MERCHANT_PURCHASE_LOG_ID        "id"
#define STARKBANK_MERCHANT_PURCHASE_LOG_CREATED   "created"
#define STARKBANK_MERCHANT_PURCHASE_LOG_TYPE      "type"
#define STARKBANK_MERCHANT_PURCHASE_LOG_ERRORS    "errors"
#define STARKBANK_MERCHANT_PURCHASE_LOG_PURCHASE  "purchase"

STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_log_params_new(
    starkbank_entity **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_log_get(
    const starkbank_client *client, const char *id, starkbank_entity **out,
    starkbank_errors **errors);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_log_query(
    const starkbank_client *client, const starkbank_entity *params, int limit,
    starkbank_iter **out);
STARKBANK_API int STARKBANK_CALL starkbank_merchant_purchase_log_page(
    const starkbank_client *client, const starkbank_entity *params, starkbank_list **out,
    char **out_cursor, starkbank_errors **errors);


#ifdef __cplusplus
}
#endif

#endif /* STARKBANK_H */
