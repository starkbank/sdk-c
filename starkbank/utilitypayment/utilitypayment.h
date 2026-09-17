/*
 * UtilityPayment, and utilitypayment.Log. Tables only.
 *
 * line and barCode are sdk-python's "conditionally required" pair - exactly
 * one identifies the utility bill being paid, and if both are sent they must
 * agree - the same shape as BoletoPayment's line/barCode and for the same
 * reason: neither gets a bare positional slot in __init__, so python is not
 * calling either required, and "exactly one of these two" is not a fact the
 * flags column has room for. A caller who sends neither or a mismatched pair
 * is told by the API, not by this table.
 *
 * scheduled is DATE, not DATE_OR_DATETIME: sdk-python calls check_date on it
 * and nothing else, and there is no scheduled-invoice-shaped alternate
 * meaning here - it is just the day the payment runs, defaulting to today.
 *
 * UtilityPayment has no update verb, so nothing here is patchable.
 */

#ifndef STARKBANK_UTILITYPAYMENT_H
#define STARKBANK_UTILITYPAYMENT_H

#define STARKBANK_UTILITYPAYMENT_FIELDS(F)                                           \
/*    wire key           type               ref                 flags            */ \
    F("description",     STRING,            NULL,  REQUIRED | CREATE)               \
    F("line",            STRING,            NULL,  CREATE)                          \
    F("barCode",         STRING,            NULL,  CREATE)                          \
    F("scheduled",       DATE,              NULL,  CREATE)                          \
    F("tags",            LIST_STRING,       NULL,  CREATE)                          \
    F("id",               STRING,           NULL,  RO)                              \
    F("status",          STRING,            NULL,  RO)                              \
    F("amount",          AMOUNT,            NULL,  RO)                              \
    F("fee",             AMOUNT,            NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)

#define STARKBANK_UTILITYPAYMENT_LOG_FIELDS(F)                                       \
    F("id",               STRING,           NULL,  RO)                               \
    F("created",          DATETIME,         NULL,  RO)                               \
    F("type",             STRING,           NULL,  RO)                               \
    F("errors",           LIST_STRING,      NULL,  RO)                               \
    F("payment",          RESOURCE,         "UtilityPayment", RO)

#endif /* STARKBANK_UTILITYPAYMENT_H */
