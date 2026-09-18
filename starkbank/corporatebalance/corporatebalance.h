/* The degenerate shape, again: one object, no id, no filters, one verb - see
 * starkbank/balance/balance.h. sdk-python's get() is the same
 * next(rest.get_stream(...)) head-of-listing trick. */

#ifndef STARKBANK_CORPORATEBALANCE_H
#define STARKBANK_CORPORATEBALANCE_H

#define STARKBANK_CORPORATE_BALANCE_FIELDS(F)                                       \
    F("id",              STRING,            NULL,  RO)                              \
    F("amount",          AMOUNT,            NULL,  RO)                              \
    F("limit",           AMOUNT,            NULL,  RO)                              \
    F("maxLimit",        AMOUNT,            NULL,  RO)                              \
    F("currency",        STRING,            NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)

#endif /* STARKBANK_CORPORATEBALANCE_H */
