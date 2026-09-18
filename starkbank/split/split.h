/*
 * Split is a resource in its own right in sdk-python, and the first slice
 * carried only the part Invoice needs: the fields that ride inside an
 * Invoice's "splits" list and the ones the API sends back in it. Split is no
 * longer Invoice-only, though: its own get/query/page now arrive too, so the
 * table below serves both a splits-list hydration and a caller reading Split
 * directly.
 *
 * The query key list is sdk-python's split.query() forwarded set: limit,
 * after, before, tags, ids, receiver_ids and status all reach
 * rest.get_stream unchanged, and page() forwards the identical set to
 * rest.get_page, so QUERY and PAGE share one table with nothing left over.
 * It matches the docs' GET /v2/split parameter list exactly (after, before,
 * cursor, fields, ids, limit, receiverIds, status, tags, once cursor and
 * fields - neither a python keyword - are set aside), so no known-drift
 * entry is needed for the query surface.
 *
 * split.Log arrived ahead of Split's own query/get/page because it was
 * reachable independently (GET /v2/split/log and GET /v2/split/log/:id do not
 * need Split.query to exist first) and SplitReceiver/SplitProfile's own logs
 * already established the shape.
 *
 * split.Log.errors is LIST_STRING even though sdk-python's Log.__init__
 * declares an errors field: the api-v2-ms-split service that backs Split,
 * SplitReceiver and SplitProfile alike never emits the key on the wire at
 * all, so the goldens carry an empty list either way, and LIST_STRING is what
 * mirrors python's plain "self.errors = errors" assignment with no coercion -
 * the same reasoning splitreceiver.h and splitprofile.h give for their own
 * Logs.
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

#define STARKBANK_SPLIT_LOG_FIELDS(F)                                                \
    F("id",              STRING,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("errors",          LIST_STRING,       NULL,  RO)                              \
    F("split",           RESOURCE,          "Split", RO)

#endif /* STARKBANK_SPLIT_H */
