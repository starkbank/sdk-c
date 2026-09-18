/*
 * MerchantCard and its Log.
 *
 * Every field on MerchantCard itself is RO: sdk-python's class docstring has
 * only "## Attributes (return-only)" and no Parameters section at all, and
 * its module has no create() - a card is stored once a MerchantSession
 * Purchase or MerchantPurchase succeeds, never posted directly. get/query/
 * page are its whole verb surface.
 *
 * expiration is DATE_OR_DATETIME: sdk-python calls check_datetime_or_date on
 * it, unlike CorporateCard's own "expiration", which the API sends as a
 * plain datetime and python leaves uncoerced.
 *
 * errors on the Log is LIST_OBJECT, the acquirer-service {code, message}
 * shape CorporatePurchase.Log already established - unlike
 * MerchantSession.Log's errors, which is a plain LIST_STRING because that
 * log is served by the acquirer's session/challenge service, not the card
 * ledger. See merchantsession.h for the other half of that distinction.
 */

#ifndef STARKBANK_MERCHANTCARD_H
#define STARKBANK_MERCHANTCARD_H

#define STARKBANK_MERCHANT_CARD_FIELDS(F)                                            \
/*    wire key           type               ref                 flags           */ \
    F("id",              STRING,            NULL,  RO)                              \
    F("ending",          STRING,            NULL,  RO)                              \
    F("fundingType",     STRING,            NULL,  RO)                              \
    F("holderName",      STRING,            NULL,  RO)                              \
    F("network",         STRING,            NULL,  RO)                              \
    F("status",          STRING,            NULL,  RO)                              \
    F("tags",            LIST_STRING,       NULL,  RO)                              \
    F("expiration",      DATE_OR_DATETIME,  NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)

#define STARKBANK_MERCHANT_CARD_LOG_FIELDS(F)                                        \
    F("id",              STRING,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("errors",          LIST_OBJECT,       NULL,  RO)                              \
    F("card",            RESOURCE,          "MerchantCard", RO)

#endif /* STARKBANK_MERCHANTCARD_H */
