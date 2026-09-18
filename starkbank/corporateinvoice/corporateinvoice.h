/*
 * CorporateInvoice. sdk-python's create() posts a single object -
 * rest.post_single(resource=_resource, entity=invoice, user=user) - so this
 * uses STARKBANK_VERB_POST_SINGLE, the Webhook shape, not POST_MULTI. There is
 * no get(): sdk-python's module defines only create(), query() and page(), so
 * this table has no GET_ID verb either.
 *
 * taxId, name, brcode, due, link, status, corporateTransactionId, updated and
 * created are all constructor keywords in sdk-python's __init__ but every one
 * of them is filed under the class docstring's "Attributes (return-only)",
 * not under "## Parameters" - only amount is required and tags is optional at
 * create - so they carry RO here and not CREATE, the same table convention
 * corporatecard.h's header already documents and drift.py's flag.conflict
 * check enforces.
 *
 * due is DATE_OR_DATETIME, not DATETIME: sdk-python coerces it with
 * check_datetime_or_date, the same coercion Invoice.due uses.
 *
 * Field order follows sdk-python's __init__ order (amount, taxId, name, tags,
 * brcode, due, link, status, corporateTransactionId, updated, created) with id
 * moved down next to updated/created, the same reshuffle corporatecard.h's
 * table already does for a CREATE-then-RO resource.
 */

#ifndef STARKBANK_CORPORATEINVOICE_H
#define STARKBANK_CORPORATEINVOICE_H

#define STARKBANK_CORPORATE_INVOICE_FIELDS(F)                                        \
/*    wire key                   type               ref             flags       */   \
    F("amount",                  AMOUNT,            NULL,  REQUIRED | CREATE)        \
    F("taxId",                   STRING,            NULL,  RO)                       \
    F("name",                    STRING,            NULL,  RO)                       \
    F("tags",                    LIST_STRING,       NULL,  CREATE)                   \
    F("brcode",                  STRING,            NULL,  RO)                       \
    F("due",                     DATE_OR_DATETIME,  NULL,  RO)                       \
    F("link",                    STRING,            NULL,  RO)                       \
    F("status",                  STRING,            NULL,  RO)                       \
    F("corporateTransactionId",  STRING,            NULL,  RO)                       \
    F("id",                      STRING,            NULL,  RO)                       \
    F("updated",                 DATETIME,          NULL,  RO)                       \
    F("created",                 DATETIME,          NULL,  RO)

#endif /* STARKBANK_CORPORATEINVOICE_H */
