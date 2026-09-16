/* The degenerate shape: one object, no id, no filters, one verb. */

#ifndef STARKBANK_BALANCE_H
#define STARKBANK_BALANCE_H

#define STARKBANK_BALANCE_FIELDS(F)                                                  \
    F("id",              STRING,            NULL,  RO)                               \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("currency",        STRING,            NULL,  RO)                               \
    F("updated",         DATETIME,          NULL,  RO)

#endif /* STARKBANK_BALANCE_H */
