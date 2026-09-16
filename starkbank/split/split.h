/*
 * Split is a resource in its own right in sdk-python, and the first slice
 * carries only the part Invoice needs: the fields that ride inside an
 * Invoice's "splits" list and the ones the API sends back in it. Its own
 * verbs (query, page, get and the SplitReceiver family) arrive with the rest
 * of the bank surface; nothing here has to change when they do.
 */

#ifndef STARKBANK_SPLIT_H
#define STARKBANK_SPLIT_H

#define STARKBANK_SPLIT_FIELDS(F)                                                    \
    F("amount",          AMOUNT,            NULL,  REQUIRED | CREATE)                \
    F("receiverId",      STRING,            NULL,  REQUIRED | CREATE)                \
    F("externalId",      STRING,            NULL,  CREATE)                           \
    F("tags",            LIST_STRING,       NULL,  CREATE)                           \
    F("scheduled",       DATETIME,          NULL,  CREATE)                           \
    F("source",          STRING,            NULL,  RO)                               \
    F("status",          STRING,            NULL,  RO)                               \
    F("id",              STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)                               \
    F("updated",         DATETIME,          NULL,  RO)

#endif /* STARKBANK_SPLIT_H */
