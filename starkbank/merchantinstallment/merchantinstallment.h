/*
 * MerchantInstallment and its Log.
 *
 * Every field is RO: sdk-python's docstring is "## Attributes (return-only)"
 * only, and the module has no create() - an installment is generated
 * automatically by the API when a MerchantPurchase is split, never posted.
 *
 * due is DATE_OR_DATETIME: sdk-python calls check_datetime_or_date on it, the
 * same coercion Invoice.due and MerchantCard.expiration use.
 *
 * purchaseIds is a real, documented query filter (query()/page() both take
 * purchase_ids) that the docs' GET /v2/merchant-installment parameter list
 * does not mention; python is normative for the verb surface, so the table
 * keeps it - see tests/reference/known-drift.json's
 * query.gone:MerchantInstallment:purchaseIds.
 *
 * errors on the Log is LIST_OBJECT, the same {code, message} shape
 * MerchantCard.Log and CorporatePurchase.Log already use.
 */

#ifndef STARKBANK_MERCHANTINSTALLMENT_H
#define STARKBANK_MERCHANTINSTALLMENT_H

#define STARKBANK_MERCHANT_INSTALLMENT_FIELDS(F)                                     \
/*    wire key           type               ref                 flags           */ \
    F("id",              STRING,            NULL,  RO)                              \
    F("amount",          AMOUNT,            NULL,  RO)                              \
    F("due",             DATE_OR_DATETIME,  NULL,  RO)                              \
    F("fee",             AMOUNT,            NULL,  RO)                              \
    F("fundingType",     STRING,            NULL,  RO)                              \
    F("network",         STRING,            NULL,  RO)                              \
    F("purchaseId",      STRING,            NULL,  RO)                              \
    F("status",          STRING,            NULL,  RO)                              \
    F("tags",            LIST_STRING,       NULL,  RO)                              \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)

#define STARKBANK_MERCHANT_INSTALLMENT_LOG_FIELDS(F)                                 \
    F("id",              STRING,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("errors",          LIST_OBJECT,       NULL,  RO)                              \
    F("installment",     RESOURCE,          "MerchantInstallment", RO)

#endif /* STARKBANK_MERCHANTINSTALLMENT_H */
