/*
 * CorporateWithdrawal. sdk-python's create() posts a single object -
 * rest.post_single(resource=_resource, entity=withdrawal, user=user) - so
 * this uses STARKBANK_VERB_POST_SINGLE, not POST_MULTI, the same shape as
 * CorporateInvoice and Webhook.
 *
 * amount and externalId are the only two "## Parameters (required)" entries
 * in sdk-python's class docstring and the only two constructor keywords with
 * no default; tags is the lone "## Parameters (optional)" entry. transactionId,
 * corporateTransactionId, updated and created are constructor keywords too but
 * are filed under "Attributes (return-only)", so they carry RO here, not
 * CREATE - the same table convention corporatecard.h and corporateinvoice.h
 * document.
 *
 * Field order follows sdk-python's __init__ order (amount, externalId, tags,
 * transactionId, corporateTransactionId, updated, created) with id moved down
 * next to updated/created, matching corporatecard.h's and corporateinvoice.h's
 * reshuffle for a CREATE-then-RO resource.
 */

#ifndef STARKBANK_CORPORATEWITHDRAWAL_H
#define STARKBANK_CORPORATEWITHDRAWAL_H

#define STARKBANK_CORPORATE_WITHDRAWAL_FIELDS(F)                                     \
/*    wire key                   type               ref             flags       */   \
    F("amount",                  AMOUNT,            NULL,  REQUIRED | CREATE)        \
    F("externalId",              STRING,            NULL,  REQUIRED | CREATE)        \
    F("tags",                    LIST_STRING,       NULL,  CREATE)                   \
    F("transactionId",           STRING,            NULL,  RO)                       \
    F("corporateTransactionId",  STRING,            NULL,  RO)                       \
    F("id",                      STRING,            NULL,  RO)                       \
    F("updated",                 DATETIME,          NULL,  RO)                       \
    F("created",                 DATETIME,          NULL,  RO)

#endif /* STARKBANK_CORPORATEWITHDRAWAL_H */
