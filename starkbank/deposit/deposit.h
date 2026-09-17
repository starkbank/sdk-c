/*
 * Deposit and deposit.Log.
 *
 * Deposit models passive cash-in received from an external transfer.
 * sdk-python's Deposit has no create() at all: every attribute is filed
 * under "## Attributes (return-only)" in the class docstring, and the only
 * mutation the API exposes is reversal - rest.patch_id with a single
 * "amount" key (payload={"amount": amount}), full or partial, where amount=0
 * fully reverses the deposit. So this table has no NEW and no create verb;
 * the only writer is STARKBANK_VERB_PATCH_ID, and amount is the only field
 * carrying PATCH.
 *
 * amount carries PATCH and neither CREATE nor REQUIRED. REQUIRED here would
 * mean "required on create" (see starkc/table.h), and this resource has no
 * create to be required for; sdk-python's update() itself defaults amount to
 * None in its signature despite the docstring's prose calling it required,
 * which is exactly the update()-is-not-create asymmetry Invoice.status and
 * BrcodePayment.status already carry. A caller who omits amount gets the
 * API's own error, not a local one invented for a distinction the flags
 * column has no room for.
 *
 * deposit.Log has a pdf verb (the reversed deposit's receipt) where
 * transfer.Log, boleto.Log, boletopayment.Log and brcodepayment.Log do not:
 * each log's pdf follows its own python module, never a blanket rule.
 */

#ifndef STARKBANK_DEPOSIT_H
#define STARKBANK_DEPOSIT_H

#define STARKBANK_DEPOSIT_FIELDS(F)                                                  \
/*    wire key           type               ref                 flags            */ \
    F("id",              STRING,            NULL,  RO)                              \
    F("name",            STRING,            NULL,  RO)                              \
    F("taxId",           STRING,            NULL,  RO)                              \
    F("bankCode",        STRING,            NULL,  RO)                              \
    F("branchCode",      STRING,            NULL,  RO)                              \
    F("accountNumber",   STRING,            NULL,  RO)                              \
    F("accountType",     STRING,            NULL,  RO)                              \
    F("amount",          AMOUNT,            NULL,  PATCH)                           \
    F("type",            STRING,            NULL,  RO)                              \
    F("status",          STRING,            NULL,  RO)                              \
    F("tags",            LIST_STRING,       NULL,  RO)                              \
    F("fee",             AMOUNT,            NULL,  RO)                              \
    F("transactionIds",  LIST_STRING,       NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("updated",         DATETIME,          NULL,  RO)

#define STARKBANK_DEPOSIT_LOG_FIELDS(F)                                             \
    F("id",              STRING,            NULL,  RO)                              \
    F("created",         DATETIME,          NULL,  RO)                              \
    F("type",            STRING,            NULL,  RO)                              \
    F("errors",          LIST_STRING,       NULL,  RO)                              \
    F("deposit",         RESOURCE,          "Deposit", RO)

#endif /* STARKBANK_DEPOSIT_H */
