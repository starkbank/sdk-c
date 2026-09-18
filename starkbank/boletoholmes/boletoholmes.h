/*
 * BoletoHolmes and its Log.
 *
 * The Log carries no errors field at all - sdk-python's Log.__init__ takes
 * (id, created, updated, type, holmes) and nothing else, the same shape
 * CorporateHolderLog and CorporateCardLog already establish for a lifecycle
 * log with no failure detail. It also carries updated, which most logs in
 * this SDK do not: sdk-python's holmes Log is itself mutated as the
 * investigation progresses, rather than being an immutable event record.
 *
 * sdk-python's page() takes only (cursor, limit, user) - none of query()'s
 * after/before/tags/ids/status/boleto_id filters - while the docs' GET
 * /v2/boleto-holmes list endpoint and query() agree on the full filter set.
 * tools/drift.py's query check compares the table's one shared query key list
 * against the docs endpoint, not against each python verb individually, so
 * this is not a finding; the table carries the full set both the docs and
 * query() agree on, and starkbank_boleto_holmes_page is simply more capable
 * than sdk-python's own page() function.
 */

#ifndef STARKBANK_BOLETOHOLMES_H
#define STARKBANK_BOLETOHOLMES_H

#define STARKBANK_BOLETO_HOLMES_FIELDS(F)                                            \
/*    wire key    type       ref     flags                */                         \
    F("boletoId", STRING,    NULL,  REQUIRED | CREATE)                               \
    F("tags",     LIST_STRING, NULL, CREATE)                                         \
    F("status",   STRING,    NULL,  RO)                                              \
    F("result",   STRING,    NULL,  RO)                                              \
    F("id",       STRING,    NULL,  RO)                                              \
    F("created",  DATETIME,  NULL,  RO)                                              \
    F("updated",  DATETIME,  NULL,  RO)

#define STARKBANK_BOLETO_HOLMES_LOG_FIELDS(F)                                        \
    F("id",       STRING,   NULL,  RO)                                               \
    F("created",  DATETIME, NULL,  RO)                                               \
    F("updated",  DATETIME, NULL,  RO)                                               \
    F("type",     STRING,   NULL,  RO)                                               \
    F("holmes",   RESOURCE, "BoletoHolmes", RO)

#endif /* STARKBANK_BOLETOHOLMES_H */
