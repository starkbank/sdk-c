/*
 * BoletoPayment, and boletopayment.Log. Tables only.
 *
 * line and bar_code are sdk-python's "conditionally required" pair - exactly
 * one identifies the boleto being paid, and if both are sent they must agree.
 * They carry CREATE and not REQUIRED for the same reason Transfer.externalId
 * does: sdk-python's __init__ gives neither a bare positional slot, so python
 * is not calling either required, and the pairing rule itself is not a fact
 * the flags column has room for - a caller who gets it wrong is told by the
 * API, not by this table.
 */

#ifndef STARKBANK_BOLETOPAYMENT_H
#define STARKBANK_BOLETOPAYMENT_H

#define STARKBANK_BOLETOPAYMENT_FIELDS(F)                                            \
/*    wire key           type               ref                 flags            */ \
    F("taxId",           STRING,            NULL,  REQUIRED | CREATE)                \
    F("description",     STRING,            NULL,  REQUIRED | CREATE)                \
    F("line",            STRING,            NULL,  CREATE)                           \
    F("barCode",         STRING,            NULL,  CREATE)                           \
    F("amount",          AMOUNT,            NULL,  CREATE)                           \
    F("scheduled",       DATE,              NULL,  CREATE)                           \
    F("tags",            LIST_STRING,       NULL,  CREATE)                           \
    F("id",              STRING,            NULL,  RO)                               \
    F("status",          STRING,            NULL,  RO)                               \
    F("fee",             AMOUNT,            NULL,  RO)                               \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_BOLETOPAYMENT_LOG_FIELDS(F)                                        \
    F("id",               STRING,            NULL,  RO)                              \
    F("created",          DATETIME,          NULL,  RO)                              \
    F("type",             STRING,            NULL,  RO)                              \
    F("errors",           LIST_STRING,       NULL,  RO)                              \
    F("payment",          RESOURCE,          "BoletoPayment", RO)

#endif /* STARKBANK_BOLETOPAYMENT_H */
