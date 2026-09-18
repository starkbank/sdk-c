/*
 * DynamicBrcode and its Rule.
 *
 * Both id and uuid are real, separate wire keys: sdk-python's __init__ stores
 * `self.uuid = uuid` in addition to the `id` Resource.__init__ already sets,
 * and get(uuid) forwards straight into rest.get_id(id=uuid) - the same shape
 * MerchantSession already establishes for id/uuid living side by side (see
 * merchantsession.h).
 *
 * starkbank_dynamic_brcode_get reaches GET /v2/dynamic-brcode/:uuid, but
 * STARKBANK_VERB_GET_ID always names its path slot ":id" - the identical
 * placeholder-naming gap tests/reference/known-drift.json's
 * endpoint.changed:MerchantSession:purchase entry already documents for this
 * exact docs pair (that entry calls out GET /v2/dynamic-brcode/:uuid by name
 * as one half of the evidence that ":uuid" and ":id" are the same kind of
 * slot spelled two ways across the docs). The route and method match; only
 * the placeholder's name differs, which is not a fact drift.py's endpoint
 * check can see through, so it is a separate accepted entry here.
 *
 * Rule.value is LIST_STRING, not STRING: sdk-python's Rule.__init__ stores it
 * with no coercion at all, but the docs' own worked example for "rules" is
 * `[{"key": "allowedTaxIds", "value": ["012.345.678-90", ...]}]` - the same
 * key/list-of-strings shape as Invoice.Rule, and there is no python type
 * declared anywhere for the table to contradict.
 */

#ifndef STARKBANK_DYNAMICBRCODE_H
#define STARKBANK_DYNAMICBRCODE_H

#define STARKBANK_DYNAMIC_BRCODE_FIELDS(F)                                           \
/*    wire key             type          ref                  flags             */   \
    F("amount",            AMOUNT,       NULL,  REQUIRED | CREATE)                   \
    F("expiration",        SECONDS,      NULL,  CREATE)                              \
    F("tags",              LIST_STRING,  NULL,  CREATE)                              \
    F("displayDescription", STRING,      NULL,  CREATE)                              \
    F("rules",             LIST_RESOURCE, "DynamicBrcode.Rule", CREATE)              \
    F("id",                STRING,       NULL,  RO)                                  \
    F("uuid",              STRING,       NULL,  RO)                                  \
    F("pictureUrl",        STRING,       NULL,  RO)                                  \
    F("updated",           DATETIME,     NULL,  RO)                                  \
    F("created",           DATETIME,     NULL,  RO)

#define STARKBANK_DYNAMIC_BRCODE_RULE_FIELDS(F)                                      \
    F("key",   STRING,      NULL, REQUIRED | CREATE)                                 \
    F("value", LIST_STRING, NULL, REQUIRED | CREATE)

#endif /* STARKBANK_DYNAMICBRCODE_H */
