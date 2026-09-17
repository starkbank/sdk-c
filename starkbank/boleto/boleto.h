/*
 * Boleto, and boleto.Log. Tables only.
 *
 * due is DATE, not DATE_OR_DATETIME: sdk-python calls check_date(due) here,
 * unlike Invoice.due's check_datetime_or_date. There is no scheduled-invoice
 * equivalent on a Boleto - a datetime would simply be rejected by check_date -
 * so the table is not free to be permissive the way Invoice's is.
 *
 * overdueLimit is a day count (max 59), not an amount or a duration sent as
 * seconds: sdk-python assigns it straight through with no check_* coercion,
 * and the docs tag it INTEGER, so it is NUMBER rather than AMOUNT or SECONDS.
 *
 * receiverName and receiverTaxId must be supplied together or not at all -
 * sdk-python's docstring says so in prose, not in the signature - and that
 * pairing is not a fact the flags column has room for. A caller who gets it
 * wrong is told by the API, not by this table.
 *
 * app-docs' POST /v2/boleto lists an optional "splits" parameter that
 * sdk-python's Boleto class does not carry as a constructor field at all.
 * Python is normative for the field set (design.md, source ranking #2), so
 * the table follows python and omits it. tools/drift.py's field checks
 * compare python against the table and never reach a create body's docs
 * listing, so this divergence raises no finding either way - it is recorded
 * here, in prose, because the checker has no channel for it.
 *
 * pdf takes two verb-specific query parameters sdk-python passes as ordinary
 * keyword arguments - layout and hiddenFields - which is a shape CONTENT and
 * CONTENT_INT do not cover (two optional string keys instead of one bounded
 * integer). hiddenFields is comma-joined by the caller rather than an array:
 * core-c's own query encoder joins a JSON array with "," before percent-
 * encoding it, so the two forms reach the wire identically, and a plain
 * const char * is the ABI shape every binding generator already translates.
 * STARKBANK_VERB_CONTENT_QUERY generalises CONTENT_INT's "0 means send
 * nothing" rule to two more string keys, in the engine rather than as a
 * per-resource hand-written function - the whole point of this build being
 * that a resource is a table, not C.
 */

#ifndef STARKBANK_BOLETO_H
#define STARKBANK_BOLETO_H

#define STARKBANK_BOLETO_FIELDS(F)                                                   \
/*    wire key           type               ref                 flags            */ \
    F("amount",          AMOUNT,            NULL,  REQUIRED | CREATE)                \
    F("name",            STRING,            NULL,  REQUIRED | CREATE)                \
    F("taxId",           STRING,            NULL,  REQUIRED | CREATE)                \
    F("streetLine1",     STRING,            NULL,  REQUIRED | CREATE)                \
    F("streetLine2",     STRING,            NULL,  REQUIRED | CREATE)                \
    F("district",        STRING,            NULL,  REQUIRED | CREATE)                \
    F("city",            STRING,            NULL,  REQUIRED | CREATE)                \
    F("stateCode",       STRING,            NULL,  REQUIRED | CREATE)                \
    F("zipCode",         STRING,            NULL,  REQUIRED | CREATE)                \
    F("due",             DATE,              NULL,  CREATE)                           \
    F("fine",            RATE,              NULL,  CREATE)                           \
    F("interest",        RATE,              NULL,  CREATE)                           \
    F("overdueLimit",    NUMBER,            NULL,  CREATE)                           \
    F("descriptions",    LIST_OBJECT,       NULL,  CREATE)                           \
    F("discounts",       LIST_OBJECT,       NULL,  CREATE)                           \
    F("tags",            LIST_STRING,       NULL,  CREATE)                           \
    F("receiverName",    STRING,            NULL,  CREATE)                           \
    F("receiverTaxId",   STRING,            NULL,  CREATE)                           \
    F("fee",             AMOUNT,            NULL,  RO)                               \
    F("line",            STRING,            NULL,  RO)                               \
    F("barCode",         STRING,            NULL,  RO)                               \
    F("status",          STRING,            NULL,  RO)                               \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                               \
    F("workspaceId",     STRING,            NULL,  RO)                               \
    F("ourNumber",       STRING,            NULL,  RO)                               \
    F("id",              STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_BOLETO_LOG_FIELDS(F)                                               \
    F("id",               STRING,            NULL,  RO)                              \
    F("created",          DATETIME,          NULL,  RO)                              \
    F("type",             STRING,            NULL,  RO)                              \
    F("errors",           LIST_STRING,       NULL,  RO)                              \
    F("boleto",           RESOURCE,          "Boleto", RO)

#endif /* STARKBANK_BOLETO_H */
