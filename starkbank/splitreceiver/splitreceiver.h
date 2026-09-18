/*
 * SplitReceiver and its Log: the bank account a Split's "receiverId" names.
 *
 * The query key list is sdk-python's query() forwarded set (limit, after,
 * before, transaction_ids, status, tax_id, sort, tags, ids - every one of
 * query()'s keyword arguments reaches rest.get_stream unchanged); page() is a
 * narrower subset (it never took transaction_ids or tax_id in its own
 * signature), and since QUERY and PAGE share one table here, query()'s fuller
 * set is what the table carries. It disagrees with the docs' GET
 * /v2/split-receiver parameter list in five places, each recorded in
 * tests/reference/known-drift.json:
 *
 *   - the docs list receiverIds; no SplitReceiver field or python keyword is
 *     named anything like it (a SplitReceiver does not itself reference
 *     another receiver) - almost certainly carried over from split.json's own
 *     receiverIds filter (Split really does have a receiverId) rather than a
 *     real SplitReceiver filter.
 *   - the docs list taxIds (plural); sdk-python's tax_id keyword is singular
 *     and camelCases to taxId, not taxIds - a naming mismatch, not a missing
 *     filter, so the table keeps python's taxId rather than the docs' plural.
 *   - the table therefore carries taxId, which the docs do not list under
 *     that exact spelling, and sort/transactionIds, which query() forwards
 *     and the docs omit entirely.
 *
 * receiver.Log's own query key list (limit, after, before, types,
 * receiverIds) matches the docs' GET /v2/split-receiver/log parameters
 * exactly and needs no exemption.
 *
 * errors on the Log is LIST_STRING: sdk-python's docstring says "list of
 * strings" and, unlike VerifiedAccount.Log, nothing in this brief overrides
 * it - the api-v2-ms-split service that backs Split/SplitReceiver/
 * SplitProfile never emits the key at all, so the goldens carry an empty
 * list and LIST_STRING is what mirrors python's plain "self.errors = errors"
 * assignment.
 */

#ifndef STARKBANK_SPLITRECEIVER_H
#define STARKBANK_SPLITRECEIVER_H

#define STARKBANK_SPLIT_RECEIVER_FIELDS(F)                                           \
/*    wire key           type               ref                 flags           */  \
    F("name",            STRING,            NULL,  REQUIRED | CREATE)                \
    F("taxId",           STRING,            NULL,  REQUIRED | CREATE)                \
    F("bankCode",        STRING,            NULL,  REQUIRED | CREATE)                \
    F("branchCode",      STRING,            NULL,  REQUIRED | CREATE)                \
    F("accountNumber",   STRING,            NULL,  REQUIRED | CREATE)                \
    F("accountType",     STRING,            NULL,  REQUIRED | CREATE)                \
    F("tags",            LIST_STRING,       NULL,  CREATE)                           \
    F("id",              STRING,            NULL,  RO)                              \
    F("status",          STRING,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)

#define STARKBANK_SPLIT_RECEIVER_LOG_FIELDS(F)                                       \
    F("id",              STRING,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("errors",          LIST_STRING,       NULL,  RO)                              \
    F("receiver",        RESOURCE,          "SplitReceiver", RO)

#endif /* STARKBANK_SPLITRECEIVER_H */
