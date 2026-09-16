/*
 * Invoice and the three sub-resources that live with it. Tables only.
 *
 * The flags column decides whether a create is validated here or 400s at the
 * API, so tools/drift.py checks it against sdk-python rather than against a
 * comment in this repo: flag.conflict derives CREATE from the class
 * docstring's Parameters/Attributes sections and REQUIRED from that plus the
 * __init__ signature, and it is a hard stop that a plain known-drift entry
 * cannot silence. PATCH is the one bit with no python counterpart - update()
 * takes **patch and names nothing - so it stays a human adjudication, and so
 * does any REQUIRED recorded with a resolution in known-drift.json.
 *
 * amount carries REQUIRED and a legal value of zero, which is the case that
 * breaks any engine that treats absent and zero the same: an Invoice with
 * amount 0 accepts whatever the payer sends.
 */

#ifndef STARKBANK_INVOICE_H
#define STARKBANK_INVOICE_H

#define STARKBANK_INVOICE_FIELDS(F)                                                  \
/*    wire key           type               ref                 flags            */ \
    F("amount",          AMOUNT,            NULL,  REQUIRED | CREATE | PATCH)        \
    F("taxId",           STRING,            NULL,  REQUIRED | CREATE)                \
    F("name",            STRING,            NULL,  REQUIRED | CREATE)                \
    F("due",             DATE_OR_DATETIME,  NULL,  CREATE | PATCH)                   \
    F("expiration",      SECONDS,           NULL,  CREATE | PATCH)                   \
    F("fine",            RATE,              NULL,  CREATE)                           \
    F("interest",        RATE,              NULL,  CREATE)                           \
    F("discounts",       LIST_OBJECT,       NULL,  CREATE)                           \
    F("descriptions",    LIST_OBJECT,       NULL,  CREATE)                           \
    F("rules",           LIST_RESOURCE,     "Invoice.Rule", CREATE)                  \
    F("splits",          LIST_RESOURCE,     "Split",        CREATE)                  \
    F("tags",            LIST_STRING,       NULL,  CREATE)                           \
    F("status",          STRING,            NULL,  PATCH)                            \
    F("pdf",             STRING,            NULL,  RO)                               \
    F("link",            STRING,            NULL,  RO)                               \
    F("brcode",          STRING,            NULL,  RO)                               \
    F("nominalAmount",   AMOUNT,            NULL,  RO)                               \
    F("fineAmount",      AMOUNT,            NULL,  RO)                               \
    F("interestAmount",  AMOUNT,            NULL,  RO)                               \
    F("discountAmount",  AMOUNT,            NULL,  RO)                               \
    F("fee",             AMOUNT,            NULL,  RO)                               \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                               \
    F("id",              STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)                               \
    F("updated",         DATETIME,          NULL,  RO)

/* value is a list of strings here and a number on Transfer.Rule, which is why
   the two carry separate tables and separate tags rather than one "Rule". */
#define STARKBANK_INVOICE_RULE_FIELDS(F)                                             \
    F("key",             STRING,            NULL,  REQUIRED | CREATE)                \
    F("value",           LIST_STRING,       NULL,  REQUIRED | CREATE)

#define STARKBANK_INVOICE_PAYMENT_FIELDS(F)                                          \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("name",            STRING,            NULL,  RO)                               \
    F("taxId",           STRING,            NULL,  RO)                               \
    F("bankCode",        STRING,            NULL,  RO)                               \
    F("branchCode",      STRING,            NULL,  RO)                               \
    F("accountNumber",   STRING,            NULL,  RO)                               \
    F("accountType",     STRING,            NULL,  RO)                               \
    F("endToEndId",      STRING,            NULL,  RO)                               \
    F("method",          STRING,            NULL,  RO)

#define STARKBANK_INVOICE_LOG_FIELDS(F)                                              \
    F("id",              STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)                               \
    F("type",            STRING,            NULL,  RO)                               \
    F("errors",          LIST_STRING,       NULL,  RO)                               \
    F("invoice",         RESOURCE,          "Invoice", RO)

#endif /* STARKBANK_INVOICE_H */
