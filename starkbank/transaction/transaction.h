/*
 * Transaction. sdk-python's create() is deprecated since v2.31.0 and
 * unconditionally raises StarkError("deprecated") - it is not a working
 * create, it is a tombstone left so old callers get a clear error rather
 * than a missing symbol. Transactions now arise only as a side effect of
 * other operations (a Transfer, a paid charge...), so this table has no NEW
 * and no create verb at all: adding one would let a C caller do, over the
 * wire, exactly what sdk-python refuses to do at all. Every field is
 * therefore RO, and the only verbs are the read ones: PARAMS, GET_ID, QUERY,
 * PAGE.
 */

#ifndef STARKBANK_TRANSACTION_H
#define STARKBANK_TRANSACTION_H

#define STARKBANK_TRANSACTION_FIELDS(F)                                             \
/*    wire key           type               ref                 flags           */ \
    F("amount",          AMOUNT,            NULL,  RO)                              \
    F("description",     STRING,            NULL,  RO)                              \
    F("externalId",      STRING,            NULL,  RO)                              \
    F("receiverId",      STRING,            NULL,  RO)                              \
    F("senderId",        STRING,            NULL,  RO)                              \
    F("tags",            LIST_STRING,       NULL,  RO)                              \
    F("id",              STRING,            NULL,  RO)                              \
    F("fee",             AMOUNT,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("source",          STRING,            NULL,  RO)                              \
    F("balance",         AMOUNT,            NULL,  RO)

#endif /* STARKBANK_TRANSACTION_H */
