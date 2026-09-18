/*
 * PaymentRequest.
 *
 * payment has no ref in its row, the same polymorphism Event.log and
 * PaymentPreview.payment already carry: which table it hydrates as is
 * written in the sibling "type" field, so the engine asks the document
 * rather than a fixed ref. The map here is sdk-python's _parse_payment,
 * verbatim, and its targets are the SDK's plain top-level resource tags
 * ("Transfer", "BoletoPayment", ...) rather than a "PaymentRequest.X"
 * qualified tag the way PaymentPreview's four previews are qualified -
 * because these seven really are the same Transfer/Transaction/BrcodePayment/
 * BoletoPayment/UtilityPayment/DarfPayment/TaxPayment objects those families
 * already register, not a PaymentRequest-owned sub-resource shape of their
 * own.
 *
 * payment is also the first CREATE-writable polymorphic RESOURCE field in
 * this SDK - Event.log and PaymentPreview.payment are both return-only, so
 * neither ever needed a writer. No new engine primitive was added for it: a
 * caller builds the underlying payment with its own family's constructor
 * (starkbank_transfer_new, starkbank_boleto_payment_new, ...), serializes it
 * with starkbank_entity_dump, and embeds it with the existing
 * starkbank_entity_set_json_raw(request, STARKBANK_PAYMENT_REQUEST_PAYMENT,
 * json) - the same escape hatch merchantsession.h's Purchase.metadata already
 * uses for its own single CREATE-writable object field. A dedicated
 * single-entity setter would buy back some type safety, but this field is
 * one of ~25 judgement cases in this build rather than the common path the
 * generic accessors are optimised for, and the existing hatch already gets a
 * caller there with zero ABI growth.
 *
 * due is STRING, not DATE_OR_DATETIME, even though it plainly carries a date:
 * sdk-python's __init__ does `self.due = due` with no check_date call at all,
 * the same gap paymentpreview.h's BoletoPreview.due/expiration already
 * documents - a table type stricter than python's real (non-)coercion would
 * be inventing a semantic tools/drift.py's type.conflict treats as a hard
 * stop, not a table improvement.
 *
 * centerId is REQUIRED on create, and also the one filter a caller MUST set
 * to query or page PaymentRequests at all: sdk-python's query()/page() both
 * take center_id as a plain positional argument with no default. The query
 * key list below carries it like any other filter because this tier's params
 * bag has no per-field REQUIRED concept for a query - only for create - so
 * the API's own 400 is what catches an omitted centerId on a list call.
 */

#ifndef STARKBANK_PAYMENTREQUEST_H
#define STARKBANK_PAYMENTREQUEST_H

#define STARKBANK_PAYMENT_REQUEST_FIELDS(F)                                          \
/*    wire key       type         ref     flags                                  */  \
    F("centerId",    STRING,      NULL,  REQUIRED | CREATE)                          \
    F("payment",     RESOURCE,    NULL,  REQUIRED | CREATE)                          \
    F("type",        STRING,      NULL,  CREATE)                                     \
    F("due",         STRING,      NULL,  CREATE)                                     \
    F("tags",        LIST_STRING, NULL,  CREATE)                                     \
    F("id",          STRING,      NULL,  RO)                                         \
    F("amount",      AMOUNT,      NULL,  RO)                                         \
    F("description", STRING,      NULL,  RO)                                         \
    F("status",      STRING,      NULL,  RO)                                         \
    F("actions",     LIST_OBJECT, NULL,  RO)                                         \
    F("updated",     DATETIME,    NULL,  RO)                                         \
    F("created",     DATETIME,    NULL,  RO)

/* sdk-python's _parse_payment dict, verbatim and in its order. A type this
   build predates (there is none today - all seven exist) resolves to NULL,
   leaving payment untagged and permissive, the same fallback Event.log and
   PaymentPreview.payment already take. */
#define STARKBANK_PAYMENT_REQUEST_VARIANTS(V)                                        \
    V("transfer",        "Transfer")                                                 \
    V("transaction",     "Transaction")                                              \
    V("boleto-payment",  "BoletoPayment")                                            \
    V("brcode-payment",  "BrcodePayment")                                            \
    V("utility-payment", "UtilityPayment")                                           \
    V("darf-payment",    "DarfPayment")                                              \
    V("tax-payment",     "TaxPayment")

#endif /* STARKBANK_PAYMENTREQUEST_H */
