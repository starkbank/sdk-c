/*
 * TaxPayment, and taxpayment.Log. Tables only.
 *
 * line and barCode are sdk-python's "conditionally required" pair, exactly
 * as they are on UtilityPayment and BoletoPayment: exactly one identifies
 * the tax slip being paid, and if both are sent they must agree. Neither
 * gets a positional slot in __init__, so neither carries REQUIRED; a caller
 * who gets the pairing wrong is told by the API.
 *
 * scheduled is DATE, not DATE_OR_DATETIME, for the same reason as
 * UtilityPayment.scheduled: sdk-python calls check_date only, and there is
 * no scheduled-invoice-shaped alternate meaning.
 *
 * TaxPayment has no update verb, so nothing here is patchable.
 */

#ifndef STARKBANK_TAXPAYMENT_H
#define STARKBANK_TAXPAYMENT_H

#define STARKBANK_TAXPAYMENT_FIELDS(F)                                               \
/*    wire key           type               ref                 flags            */ \
    F("description",     STRING,            NULL,  REQUIRED | CREATE)               \
    F("line",            STRING,            NULL,  CREATE)                          \
    F("barCode",         STRING,            NULL,  CREATE)                          \
    F("scheduled",       DATE,              NULL,  CREATE)                          \
    F("tags",            LIST_STRING,       NULL,  CREATE)                          \
    F("id",               STRING,           NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("status",          STRING,            NULL,  RO)                              \
    F("amount",          AMOUNT,            NULL,  RO)                              \
    F("fee",             AMOUNT,            NULL,  RO)                              \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_TAXPAYMENT_LOG_FIELDS(F)                                           \
    F("id",               STRING,           NULL,  RO)                               \
    F("created",          DATETIME,         NULL,  RO)                               \
    F("type",             STRING,           NULL,  RO)                               \
    F("errors",           LIST_STRING,      NULL,  RO)                               \
    F("payment",          RESOURCE,         "TaxPayment", RO)

#endif /* STARKBANK_TAXPAYMENT_H */
