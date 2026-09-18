/*
 * MerchantPurchase and its Log.
 *
 * create() is post_single, the same shape as Webhook and Workspace: python's
 * rest.post_single sends the MerchantPurchase itself as the body, no plural
 * envelope, no list. This is the family-level Purchase counterpart to
 * MerchantSession's Purchase sub-resource (see merchantsession.h): a
 * MerchantPurchase can be posted directly with full card data (card_id plus
 * card_expiration/card_number/card_security_code/holder_name when not
 * created through a session) or reference a MerchantSession Purchase's
 * card_id after 3DS succeeds - either way it is its own top-level resource
 * with its own id, table and verbs, never reached through another
 * resource's sub-path.
 *
 * update(id, status, amount) sends exactly {"status": status, "amount":
 * amount} - see sdk-python's docstring: status is "canceled" (with amount=0)
 * to cancel an approved purchase, or "reversed" with a lower amount to
 * partially or fully reverse a confirmed one. Both keys are PATCH here and
 * nowhere else on this table, matching that payload exactly.
 *
 * holderId is a real, documented query filter (query()/page() both take
 * holder_id) the docs' GET /v2/merchant-purchase parameter list omits;
 * python is normative for the verb surface - see
 * tests/reference/known-drift.json's query.gone:MerchantPurchase:holderId.
 *
 * errors on the Log is LIST_OBJECT, the same {code, message} shape
 * MerchantCard.Log and MerchantInstallment.Log already use.
 */

#ifndef STARKBANK_MERCHANTPURCHASE_H
#define STARKBANK_MERCHANTPURCHASE_H

#define STARKBANK_MERCHANT_PURCHASE_FIELDS(F)                                         \
/*    wire key                type         ref     flags                          */ \
    F("amount",                AMOUNT,      NULL,  REQUIRED | CREATE | PATCH)          \
    F("cardId",                STRING,      NULL,  REQUIRED | CREATE)                  \
    F("fundingType",           STRING,      NULL,  REQUIRED | CREATE)                  \
    F("installmentCount",      NUMBER,      NULL,  REQUIRED | CREATE)                  \
    F("cardExpiration",        STRING,      NULL,  CREATE)                             \
    F("cardNumber",            STRING,      NULL,  CREATE)                             \
    F("cardSecurityCode",      STRING,      NULL,  CREATE)                             \
    F("holderName",            STRING,      NULL,  CREATE)                             \
    F("holderEmail",           STRING,      NULL,  CREATE)                             \
    F("holderPhone",           STRING,      NULL,  CREATE)                             \
    F("holderId",              STRING,      NULL,  CREATE)                             \
    F("billingCountryCode",    STRING,      NULL,  CREATE)                             \
    F("billingCity",           STRING,      NULL,  CREATE)                             \
    F("billingStateCode",      STRING,      NULL,  CREATE)                             \
    F("billingStreetLine1",    STRING,      NULL,  CREATE)                             \
    F("billingStreetLine2",    STRING,      NULL,  CREATE)                             \
    F("billingZipCode",        STRING,      NULL,  CREATE)                             \
    F("metadata",              OBJECT,      NULL,  CREATE)                             \
    F("softDescriptor",        STRING,      NULL,  CREATE)                             \
    F("tags",                  LIST_STRING, NULL,  CREATE)                             \
    F("id",                    STRING,      NULL,  RO)                                 \
    F("cardEnding",            STRING,      NULL,  RO)                                 \
    F("challengeMode",         STRING,      NULL,  RO)                                 \
    F("challengeUrl",          STRING,      NULL,  RO)                                 \
    F("currencyCode",          STRING,      NULL,  RO)                                 \
    F("endToEndId",            STRING,      NULL,  RO)                                 \
    F("fee",                   AMOUNT,      NULL,  RO)                                 \
    F("network",                STRING,     NULL,  RO)                                 \
    F("source",                 STRING,     NULL,  RO)                                 \
    F("status",                 STRING,     NULL,  PATCH)                              \
    F("created",                 DATETIME,  NULL,  RO)                                 \
    F("updated",                 DATETIME,  NULL,  RO)

#define STARKBANK_MERCHANT_PURCHASE_LOG_FIELDS(F)                                     \
    F("id",              STRING,            NULL,  RO)                                \
    F("created",         DATETIME,          NULL,  RO)                                \
    F("type",            STRING,            NULL,  RO)                                \
    F("errors",          LIST_OBJECT,       NULL,  RO)                                \
    F("purchase",        RESOURCE,          "MerchantPurchase", RO)

#endif /* STARKBANK_MERCHANTPURCHASE_H */
