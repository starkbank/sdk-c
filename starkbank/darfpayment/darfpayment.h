/*
 * DarfPayment, and darfpayment.Log. Tables only.
 *
 * Unlike UtilityPayment and TaxPayment, a DarfPayment is not identified by a
 * line or a bar code: it is fully structured, so every one of its create
 * fields is REQUIRED rather than a conditionally-required pair. It also has
 * no "type" attribute - sdk-python's DarfPayment carries none, where
 * UtilityPayment and TaxPayment both do.
 *
 * competence and due are DATE, not DATE_OR_DATETIME: sdk-python calls
 * check_date on both, the same as scheduled, and neither carries the
 * scheduled-invoice-shaped alternate meaning Invoice.due has.
 *
 * DarfPayment has no update verb, so nothing here is patchable.
 */

#ifndef STARKBANK_DARFPAYMENT_H
#define STARKBANK_DARFPAYMENT_H

#define STARKBANK_DARFPAYMENT_FIELDS(F)                                              \
/*    wire key             type              ref                 flags          */ \
    F("description",       STRING,           NULL,  REQUIRED | CREATE)              \
    F("revenueCode",        STRING,           NULL,  REQUIRED | CREATE)             \
    F("taxId",              STRING,           NULL,  REQUIRED | CREATE)             \
    F("competence",         DATE,             NULL,  REQUIRED | CREATE)             \
    F("nominalAmount",      AMOUNT,           NULL,  REQUIRED | CREATE)             \
    F("fineAmount",         AMOUNT,           NULL,  REQUIRED | CREATE)             \
    F("interestAmount",     AMOUNT,           NULL,  REQUIRED | CREATE)             \
    F("due",                DATE,             NULL,  REQUIRED | CREATE)             \
    F("referenceNumber",    STRING,           NULL,  CREATE)                        \
    F("scheduled",          DATE,             NULL,  CREATE)                        \
    F("tags",               LIST_STRING,      NULL,  CREATE)                       \
    F("id",                 STRING,           NULL,  RO)                           \
    F("status",             STRING,           NULL,  RO)                           \
    F("amount",             AMOUNT,           NULL,  RO)                           \
    F("fee",                AMOUNT,           NULL,  RO)                           \
    F("transactionIds",     LIST_STRING,      NULL,  RO)                           \
    F("updated",            DATETIME,         NULL,  RO)                           \
    F("created",            DATETIME,         NULL,  RO)

#define STARKBANK_DARFPAYMENT_LOG_FIELDS(F)                                          \
    F("id",               STRING,           NULL,  RO)                              \
    F("created",          DATETIME,         NULL,  RO)                              \
    F("type",             STRING,           NULL,  RO)                              \
    F("errors",           LIST_STRING,      NULL,  RO)                              \
    F("payment",          RESOURCE,         "DarfPayment", RO)

#endif /* STARKBANK_DARFPAYMENT_H */
