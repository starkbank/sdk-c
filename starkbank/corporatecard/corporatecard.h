/*
 * CorporateCard and its Log.
 *
 * create() posts to "corporate-card/token", not "corporate-card": sdk-python's
 * rest.post_raw builds that path by hand and unwraps the reply with
 * last_name(_resource) - CorporateCard's own singular key, "card" - rather
 * than a "token" resource's. No existing engine verb macro is that shape (see
 * starkbankVerbCreateSub's comment in starkc/verb.c), so this table uses the
 * new STARKBANK_VERB_POST_SINGLE_SUB macro instead of STARKBANK_VERB_POST_SINGLE.
 *
 * Only holderId carries CREATE: sdk-python's create() docstring requires just
 * holder_id, and the documented POST /v2/corporate-card/token body accepts
 * holderId, merchantId and merchantName - none of which this table's other
 * fields are, since sdk-python's CorporateCard class does not carry merchant
 * fields at all. Every other field is filed under the class docstring's
 * "Attributes (return-only)", matching the table convention drift.py's
 * flag.conflict check itself uses.
 *
 * displayName, rules, tags, status and pin are PATCH: sdk-python's update()
 * sends exactly these five in its payload dict. pin has no python __init__
 * counterpart at all - it unlocks a physical card's PIN and is never read
 * back - so it is RO nowhere and CREATE nowhere, only PATCH; see
 * known-drift.json's field.gone:CorporateCard:pin, the same shape as sdk-c4's
 * Workspace.picture.
 *
 * number, securityCode and expiration come back masked unless the caller asks
 * for expand; this build does not model create()/get()'s expand keyword for
 * the same reason CorporateHolder's header explains, though query() and
 * page() forward it for free through "expand" in the query key list below.
 */

#ifndef STARKBANK_CORPORATECARD_H
#define STARKBANK_CORPORATECARD_H

#define STARKBANK_CORPORATE_CARD_FIELDS(F)                                          \
/*    wire key           type               ref               flags            */ \
    F("holderId",        STRING,            NULL,  REQUIRED | CREATE)              \
    F("holderName",      STRING,            NULL,  RO)                             \
    F("displayName",     STRING,            NULL,  PATCH)                          \
    F("rules",           LIST_RESOURCE,     "CorporateRule", PATCH)                 \
    F("tags",            LIST_STRING,       NULL,  PATCH)                          \
    F("pin",             STRING,            NULL,  PATCH)                          \
    F("streetLine1",     STRING,            NULL,  RO)                             \
    F("streetLine2",     STRING,            NULL,  RO)                             \
    F("district",        STRING,            NULL,  RO)                             \
    F("city",            STRING,            NULL,  RO)                             \
    F("stateCode",       STRING,            NULL,  RO)                             \
    F("zipCode",         STRING,            NULL,  RO)                             \
    F("type",            STRING,            NULL,  RO)                             \
    F("status",          STRING,            NULL,  PATCH)                          \
    F("number",          STRING,            NULL,  RO)                             \
    F("securityCode",    STRING,            NULL,  RO)                             \
    F("expiration",      DATETIME,          NULL,  RO)                             \
    F("id",              STRING,            NULL,  RO)                             \
    F("updated",         DATETIME,          NULL,  RO)                             \
    F("created",         DATETIME,          NULL,  RO)

#define STARKBANK_CORPORATE_CARD_LOG_FIELDS(F)                                      \
    F("id",              STRING,            NULL,  RO)                             \
    F("created",         DATETIME,          NULL,  RO)                             \
    F("type",            STRING,            NULL,  RO)                             \
    F("card",            RESOURCE,          "CorporateCard", RO)

#endif /* STARKBANK_CORPORATECARD_H */
