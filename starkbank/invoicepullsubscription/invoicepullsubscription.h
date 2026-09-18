/*
 * InvoicePullSubscription and its Log.
 *
 * due and end are STRING, not DATE_OR_DATETIME, even though sdk-python's own
 * checks.check_datetime_or_date is exactly what the wire value goes through.
 * python spells the call `None if due == "" else check_datetime_or_date(due)`
 * (and the same shape for end) - a conditional expression, not the bare
 * `check_datetime_or_date(x)` call tools/drift.py's ast reader looks for - so
 * the checker sees no coercion at all on these two fields and a
 * DATE_OR_DATETIME table type is flagged type.conflict against that. The
 * runtime behaviour is genuinely DATE_OR_DATETIME; the divergence is a gap in
 * what the checker's simple ast.Call match can see through an IfExp, not a
 * real disagreement about the wire, and is recorded in known-drift.json
 * rather than worked around by inventing a semantic the table would then own
 * incorrectly. start carries no such guard and is a direct call, so it is
 * plain DATE_OR_DATETIME with no exemption needed.
 *
 * errors on the Log is LIST_OBJECT, not LIST_STRING, even though sdk-python's
 * own docstring says "list of strings": invoicepullsubscription.Log.__init__
 * does `self.errors = errors` with no coercion at all, so python merely
 * forwards whatever the api-v2-ms-invoice-pull backend sends, and that
 * service answers with {code, message} objects - the same shape
 * CorporatePurchase.Log's errors already carries. The docstring is stale
 * prose describing an earlier wire shape; the table follows the service.
 *
 * amount and amountMinLimit are each individually optional in sdk-python's
 * signature (both default None) even though the docstring files them under
 * "## Parameters (conditionally required)": at least one of the two is
 * required by the API, but that is a cross-field rule this tier's per-field
 * REQUIRED bit cannot express, so neither carries REQUIRED and a caller who
 * omits both gets the API's 400 rather than a local refusal that would be
 * wrong for the caller who set the other one.
 */

#ifndef STARKBANK_INVOICEPULLSUBSCRIPTION_H
#define STARKBANK_INVOICEPULLSUBSCRIPTION_H

#define STARKBANK_INVOICE_PULL_SUBSCRIPTION_FIELDS(F)                                \
/*    wire key              type               ref     flags                     */ \
    F("start",              DATE_OR_DATETIME,  NULL,  REQUIRED | CREATE)             \
    F("interval",           STRING,            NULL,  REQUIRED | CREATE)             \
    F("pullMode",           STRING,            NULL,  REQUIRED | CREATE)             \
    F("pullRetryLimit",     NUMBER,            NULL,  REQUIRED | CREATE)             \
    F("type",               STRING,            NULL,  REQUIRED | CREATE)             \
    F("amount",             AMOUNT,            NULL,  CREATE)                        \
    F("amountMinLimit",     AMOUNT,            NULL,  CREATE)                        \
    F("displayDescription", STRING,            NULL,  CREATE)                        \
    F("due",                STRING,            NULL,  CREATE)                        \
    F("externalId",         STRING,            NULL,  CREATE)                        \
    F("referenceCode",      STRING,            NULL,  CREATE)                        \
    F("end",                STRING,            NULL,  CREATE)                        \
    F("data",               OBJECT,            NULL,  CREATE)                        \
    F("name",               STRING,            NULL,  CREATE)                        \
    F("taxId",              STRING,            NULL,  CREATE)                        \
    F("tags",               LIST_STRING,       NULL,  CREATE)                        \
    F("status",             STRING,            NULL,  RO)                            \
    F("bacenId",            STRING,            NULL,  RO)                            \
    F("brcode",             STRING,            NULL,  RO)                            \
    F("id",                 STRING,            NULL,  RO)                            \
    F("created",            DATETIME,          NULL,  RO)                            \
    F("updated",            DATETIME,          NULL,  RO)

#define STARKBANK_INVOICE_PULL_SUBSCRIPTION_LOG_FIELDS(F)                            \
    F("id",           STRING,     NULL,  RO)                                         \
    F("created",      DATETIME,   NULL,  RO)                                         \
    F("type",         STRING,     NULL,  RO)                                         \
    F("errors",       LIST_OBJECT, NULL, RO)                                         \
    F("subscription", RESOURCE,   "InvoicePullSubscription", RO)

#endif /* STARKBANK_INVOICEPULLSUBSCRIPTION_H */
