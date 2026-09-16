/*
 * PaymentPreview and the four previews it hydrates into.
 *
 * payment has no ref in its row: which preview table it hydrates as is
 * written in the sibling "type" field, so the resource carries the same
 * variant map Event.log carries and the engine asks the document. That is the
 * whole of the polymorphism here - no function, no per-resource code, one
 * table of four rows.
 *
 * The map is sdk-python's _sub_resource_by_type, verbatim and in its order.
 * A type this build predates resolves to NULL, which leaves the payment
 * untagged, permissive and counted as one unknown - the same path python
 * takes, where an unmapped type leaves self.payment the raw dict.
 *
 * The preview tags are spelled "PaymentPreview.<Class>" because that is what
 * sdk-python's package layout says they are: a _sub_resource named
 * BrcodePreview inside the paymentpreview package, which is how tools/drift.py
 * joins a sub-resource to its python class and how Invoice.Rule is already
 * spelled here.
 */

#ifndef STARKBANK_PAYMENT_PREVIEW_H
#define STARKBANK_PAYMENT_PREVIEW_H

#define STARKBANK_PAYMENT_PREVIEW_FIELDS(F)                                          \
/*    wire key           type               ref                 flags            */ \
    F("id",              STRING,            NULL,  REQUIRED | CREATE)                \
    F("scheduled",       DATE,              NULL,  CREATE)                           \
    F("type",            STRING,            NULL,  RO)                               \
    F("payment",         RESOURCE,          NULL,  RO)

#define STARKBANK_PAYMENT_PREVIEW_VARIANTS(V)                                        \
    V("brcode-payment",  "PaymentPreview.BrcodePreview")                             \
    V("boleto-payment",  "PaymentPreview.BoletoPreview")                             \
    V("utility-payment", "PaymentPreview.UtilityPreview")                            \
    V("tax-payment",     "PaymentPreview.TaxPreview")

/* Return-only, every one of them: a preview is what the API answers with. */
#define STARKBANK_BRCODE_PREVIEW_FIELDS(F)                                           \
    F("status",          STRING,            NULL,  RO)                               \
    F("name",            STRING,            NULL,  RO)                               \
    F("taxId",           STRING,            NULL,  RO)                               \
    F("bankCode",        STRING,            NULL,  RO)                               \
    F("accountType",     STRING,            NULL,  RO)                               \
    F("allowChange",     BOOL,              NULL,  RO)                               \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("nominalAmount",   AMOUNT,            NULL,  RO)                               \
    F("interestAmount",  AMOUNT,            NULL,  RO)                               \
    F("fineAmount",      AMOUNT,            NULL,  RO)                               \
    F("reductionAmount", AMOUNT,            NULL,  RO)                               \
    F("discountAmount",  AMOUNT,            NULL,  RO)                               \
    F("reconciliationId", STRING,           NULL,  RO)                               \
    F("description",     STRING,            NULL,  RO)

/*
 * due and expiration are STRING and not DATE on purpose. sdk-python's
 * BoletoPreview is a SubResource whose __init__ runs no check_date, so the
 * value a python caller holds is the ISO string the API sent, and a table
 * that declared a coercion python does not perform would be inventing a
 * semantic - which is exactly what drift.py's type.conflict treats as a hard
 * stop. If sdk-python ever coerces them, this table follows and the checker
 * will say so.
 */
#define STARKBANK_BOLETO_PREVIEW_FIELDS(F)                                           \
    F("status",          STRING,            NULL,  RO)                               \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("discountAmount",  AMOUNT,            NULL,  RO)                               \
    F("fineAmount",      AMOUNT,            NULL,  RO)                               \
    F("interestAmount",  AMOUNT,            NULL,  RO)                               \
    F("due",             STRING,            NULL,  RO)                               \
    F("expiration",      STRING,            NULL,  RO)                               \
    F("name",            STRING,            NULL,  RO)                               \
    F("taxId",           STRING,            NULL,  RO)                               \
    F("receiverName",    STRING,            NULL,  RO)                               \
    F("receiverTaxId",   STRING,            NULL,  RO)                               \
    F("payerName",       STRING,            NULL,  RO)                               \
    F("payerTaxId",      STRING,            NULL,  RO)                               \
    F("line",            STRING,            NULL,  RO)                               \
    F("barCode",         STRING,            NULL,  RO)

#define STARKBANK_TAX_PREVIEW_FIELDS(F)                                              \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("name",            STRING,            NULL,  RO)                               \
    F("description",     STRING,            NULL,  RO)                               \
    F("line",            STRING,            NULL,  RO)                               \
    F("barCode",         STRING,            NULL,  RO)

#define STARKBANK_UTILITY_PREVIEW_FIELDS(F)                                          \
    F("amount",          AMOUNT,            NULL,  RO)                               \
    F("name",            STRING,            NULL,  RO)                               \
    F("description",     STRING,            NULL,  RO)                               \
    F("line",            STRING,            NULL,  RO)                               \
    F("barCode",         STRING,            NULL,  RO)

#endif /* STARKBANK_PAYMENT_PREVIEW_H */
