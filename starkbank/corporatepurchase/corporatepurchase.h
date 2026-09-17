/*
 * CorporatePurchase and its Log. Every field is RO: sdk-python's module has
 * no create() (a purchase is authorized by the card network, not posted by a
 * caller) and no update() either, even though the docs show a documented
 * PATCH /v2/corporate-purchase/:id - python is normative for the verb
 * surface, so this table has no PATCH_ID verb and no PATCH bit to go with it.
 *
 * errors on the Log is LIST_OBJECT, not LIST_STRING: corporatepurchase's
 * Log.errors is a list of {code, message} StarkCore.Error objects on the
 * wire, unlike CorporateCard.Log and CorporateHolder.Log, which the API sends
 * with no errors field at all.
 *
 * parse() and response() are hand-written in handwritten/corporatepurchase_
 * parse.c and handwritten/corporatepurchase_response.c - see those files for
 * why each needed to be, rather than fitting an existing verb macro.
 */

#ifndef STARKBANK_CORPORATEPURCHASE_H
#define STARKBANK_CORPORATEPURCHASE_H

#define STARKBANK_CORPORATE_PURCHASE_FIELDS(F)                                       \
/*    wire key                    type       ref     flags                       */ \
    F("holderId",                 STRING,    NULL,  RO)                              \
    F("holderName",               STRING,    NULL,  RO)                              \
    F("centerId",                 STRING,    NULL,  RO)                              \
    F("cardId",                   STRING,    NULL,  RO)                              \
    F("cardEnding",               STRING,    NULL,  RO)                              \
    F("description",              STRING,    NULL,  RO)                              \
    F("amount",                   AMOUNT,    NULL,  RO)                              \
    F("tax",                      AMOUNT,    NULL,  RO)                              \
    F("issuerAmount",             AMOUNT,    NULL,  RO)                              \
    F("issuerCurrencyCode",       STRING,    NULL,  RO)                              \
    F("issuerCurrencySymbol",     STRING,    NULL,  RO)                              \
    F("merchantAmount",           AMOUNT,    NULL,  RO)                              \
    F("merchantCurrencyCode",     STRING,    NULL,  RO)                              \
    F("merchantCurrencySymbol",   STRING,    NULL,  RO)                              \
    F("merchantCategoryCode",     STRING,    NULL,  RO)                              \
    F("merchantCategoryType",     STRING,    NULL,  RO)                              \
    F("merchantCountryCode",      STRING,    NULL,  RO)                              \
    F("merchantName",             STRING,    NULL,  RO)                              \
    F("merchantDisplayName",      STRING,    NULL,  RO)                              \
    F("merchantDisplayUrl",       STRING,    NULL,  RO)                              \
    F("merchantFee",              AMOUNT,    NULL,  RO)                              \
    F("methodCode",               STRING,    NULL,  RO)                              \
    F("tags",                     LIST_STRING, NULL, RO)                             \
    F("corporateTransactionIds",  LIST_STRING, NULL, RO)                             \
    F("status",                   STRING,    NULL,  RO)                              \
    F("id",                       STRING,    NULL,  RO)                              \
    F("updated",                  DATETIME,  NULL,  RO)                              \
    F("created",                  DATETIME,  NULL,  RO)

#define STARKBANK_CORPORATE_PURCHASE_LOG_FIELDS(F)                                   \
    F("id",                     STRING,    NULL,  RO)                                \
    F("created",                DATETIME,  NULL,  RO)                                \
    F("type",                   STRING,    NULL,  RO)                                \
    F("errors",                 LIST_OBJECT, NULL, RO)                               \
    F("description",            STRING,    NULL,  RO)                                \
    F("corporateTransactionId", STRING,    NULL,  RO)                                \
    F("purchase",               RESOURCE,  "CorporatePurchase", RO)

#endif /* STARKBANK_CORPORATEPURCHASE_H */
