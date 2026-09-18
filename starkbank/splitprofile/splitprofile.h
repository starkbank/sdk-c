/*
 * SplitProfile and its Log: the per-workspace default that decides how a
 * Split's amount is scheduled for transfer once it is created.
 *
 * put, not create: sdk-python's only write verb is put(splitProfile), over
 * rest.put_multi ("create it if you have not, update it if you have"), which
 * is why this is the table STARKBANK_VERB_PUT_MULTI exists for - see
 * starkc/verb.c's starkbankVerbPutMulti and the "feat: add the PUT_MULTI verb
 * shape" commit. There is no delete/cancel either: sdk-python's module has
 * none.
 *
 * delay and interval are both REQUIRED and CREATE despite sdk-python's own
 * docstring filing them under "## Parameters (optional)": SplitProfile.
 * __init__(self, delay, interval, tags=None, ...) takes both positionally
 * with no default, so constructing one without them raises TypeError - the
 * docstring's wording, not the signature, is the bug, the same shape
 * tests/reference/known-drift.json's flag.conflict:Split:externalId/tags/
 * scheduled already established for this codebase (code over docstring
 * prose). Both known-drift entries below resolve to "table".
 *
 * delay is NUMBER, not SECONDS, even though the docstring calls it "DateInterval
 * or integer" the way Invoice.expiration and MerchantSession.expiration are
 * worded: __init__ does "self.delay = delay" with no check_timedelta call at
 * all, so python sends back exactly whatever integer it was given - the same
 * type.conflict reasoning MerchantSession.expiration's header already spells
 * out, and the table follows the real coercion rather than the docstring.
 *
 * The query key list is limit/after/before/ids/receiverIds/status/tags:
 * sdk-python's query() signature accepts transaction_ids, status, tax_id,
 * sort, tags and ids as keyword arguments but its rest.get_stream call
 * forwards only limit, after, before and user - the rest are dead
 * parameters, evidently copied from SplitReceiver.query()'s signature and
 * never wired up. page(), by contrast, really does forward receiver_ids to
 * rest.get_page alongside tags, ids and status - it is a live parameter in
 * page()'s own signature, not the dead cruft query() carries, so the table
 * keeps it even though the docs' GET /v2/split-profile parameter list omits
 * it - recorded as query.gone:SplitProfile:receiverIds in
 * tests/reference/known-drift.json, the same shape as
 * query.gone:SplitReceiver:sort. limit, after, before, ids, status and tags
 * are, byte for byte, the rest of the docs' parameter list.
 *
 * errors on the Log is LIST_STRING, the same shape as SplitReceiver.Log's -
 * the api-v2-ms-split service never emits the key.
 */

#ifndef STARKBANK_SPLITPROFILE_H
#define STARKBANK_SPLITPROFILE_H

#define STARKBANK_SPLIT_PROFILE_FIELDS(F)                                            \
/*    wire key       type               ref                 flags               */  \
    F("delay",       NUMBER,            NULL,  REQUIRED | CREATE)                    \
    F("interval",    STRING,            NULL,  REQUIRED | CREATE)                    \
    F("tags",        LIST_STRING,       NULL,  CREATE)                               \
    F("id",          STRING,            NULL,  RO)                                   \
    F("status",      STRING,            NULL,  RO)                                   \
    F("created",     DATETIME,          NULL,  RO)                                   \
    F("updated",     DATETIME,          NULL,  RO)

#define STARKBANK_SPLIT_PROFILE_LOG_FIELDS(F)                                        \
    F("id",          STRING,            NULL,  RO)                                   \
    F("created",     DATETIME,          NULL,  RO)                                   \
    F("type",        STRING,            NULL,  RO)                                   \
    F("errors",      LIST_STRING,       NULL,  RO)                                   \
    F("profile",     RESOURCE,          "SplitProfile", RO)

#endif /* STARKBANK_SPLITPROFILE_H */
