/*
 * InvoicePullRequest and its Log.
 *
 * due is DATE_OR_DATETIME: unlike InvoicePullSubscription.due (see
 * invoicepullsubscription.h), sdk-python's InvoicePullRequest.__init__ does a
 * bare `self.due = check_datetime_or_date(due)` with no conditional guard, so
 * the direct-call shape tools/drift.py's ast reader expects is exactly what
 * is there and no exemption is needed.
 *
 * errors on the Log is LIST_OBJECT, not LIST_STRING, for the same reason as
 * invoicepullsubscription.Log's errors: __init__ does `self.errors = errors`
 * with no coercion, so python forwards whatever the api-v2-ms-invoice-pull
 * backend sends, which is {code, message} objects - the docstring's "list of
 * strings" is stale prose, not the real wire shape.
 */

#ifndef STARKBANK_INVOICEPULLREQUEST_H
#define STARKBANK_INVOICEPULLREQUEST_H

#define STARKBANK_INVOICE_PULL_REQUEST_FIELDS(F)                                     \
/*    wire key              type               ref     flags                     */ \
    F("subscriptionId",     STRING,            NULL,  REQUIRED | CREATE)             \
    F("invoiceId",          STRING,            NULL,  REQUIRED | CREATE)             \
    F("due",                DATE_OR_DATETIME,  NULL,  REQUIRED | CREATE)             \
    F("attemptType",        STRING,            NULL,  CREATE)                        \
    F("tags",               LIST_STRING,       NULL,  CREATE)                        \
    F("externalId",         STRING,            NULL,  CREATE)                        \
    F("displayDescription", STRING,            NULL,  CREATE)                        \
    F("status",             STRING,            NULL,  RO)                            \
    F("installmentId",      STRING,            NULL,  RO)                            \
    F("id",                 STRING,            NULL,  RO)                            \
    F("created",            DATETIME,          NULL,  RO)                            \
    F("updated",            DATETIME,          NULL,  RO)

#define STARKBANK_INVOICE_PULL_REQUEST_LOG_FIELDS(F)                                 \
    F("id",       STRING,      NULL,  RO)                                            \
    F("created",  DATETIME,    NULL,  RO)                                            \
    F("type",     STRING,      NULL,  RO)                                            \
    F("errors",   LIST_OBJECT, NULL,  RO)                                            \
    F("request",  RESOURCE,    "InvoicePullRequest", RO)

#endif /* STARKBANK_INVOICEPULLREQUEST_H */
