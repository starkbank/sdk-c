/*
 * Transfer, and the reason it is in the first slice: it must need zero
 * hand-written code beyond these tables. If it ever does, the engine is wrong
 * and the whole table-driven bet is off.
 *
 * externalId is the idempotency key the standards require of a funds-movement
 * call. It is CREATE and not REQUIRED because python does not require it, and
 * python is normative for the surface - the header says loudly that a caller
 * should always set it anyway.
 *
 * metadata is the field app-docs' spec does not model at all. Python and Go
 * both carry it, so the table does too; it is what makes the permissive-read
 * counter meaningful on a real resource rather than on a test fixture.
 */

#ifndef STARKBANK_TRANSFER_H
#define STARKBANK_TRANSFER_H

#define STARKBANK_TRANSFER_FIELDS(F)                                                 \
/*    wire key            type               ref                 flags           */ \
    F("amount",           AMOUNT,            NULL,  REQUIRED | CREATE)               \
    F("name",             STRING,            NULL,  REQUIRED | CREATE)               \
    F("taxId",            STRING,            NULL,  REQUIRED | CREATE)               \
    F("bankCode",         STRING,            NULL,  REQUIRED | CREATE)               \
    F("branchCode",       STRING,            NULL,  REQUIRED | CREATE)               \
    F("accountNumber",    STRING,            NULL,  REQUIRED | CREATE)               \
    F("accountType",      STRING,            NULL,  CREATE)                          \
    F("externalId",       STRING,            NULL,  CREATE)                          \
    F("scheduled",        DATE_OR_DATETIME,  NULL,  CREATE)                          \
    F("description",      STRING,            NULL,  CREATE)                          \
    F("displayDescription", STRING,          NULL,  CREATE)                          \
    F("tags",             LIST_STRING,       NULL,  CREATE)                          \
    F("rules",            LIST_RESOURCE,     "Transfer.Rule", CREATE)                \
    F("fee",              AMOUNT,            NULL,  RO)                              \
    F("status",           STRING,            NULL,  RO)                              \
    F("transactionIds",   LIST_STRING,       NULL,  RO)                              \
    F("metadata",         OBJECT,            NULL,  RO)                              \
    F("id",               STRING,            NULL,  RO)                              \
    F("created",          DATETIME,          NULL,  RO)                              \
    F("updated",          DATETIME,          NULL,  RO)

#define STARKBANK_TRANSFER_RULE_FIELDS(F)                                            \
    F("key",              STRING,            NULL,  REQUIRED | CREATE)               \
    F("value",            NUMBER,            NULL,  REQUIRED | CREATE)

#define STARKBANK_TRANSFER_LOG_FIELDS(F)                                             \
    F("id",               STRING,            NULL,  RO)                              \
    F("created",          DATETIME,          NULL,  RO)                              \
    F("type",             STRING,            NULL,  RO)                              \
    F("errors",           LIST_STRING,       NULL,  RO)                              \
    F("transfer",         RESOURCE,          "Transfer", RO)

#endif /* STARKBANK_TRANSFER_H */
