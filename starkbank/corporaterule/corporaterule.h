/*
 * CorporateRule: a spending rule embedded in CorporateHolder.rules and
 * CorporateCard.rules. sdk-python's corporaterule/__corporaterule.py assigns
 * a plain _resource (not _sub_resource), so - like Split - it is a top-level
 * bare resource despite having no REST verbs of its own: nothing in python
 * calls create/get/query/page on a CorporateRule directly.
 *
 * categories, countries and methods are LIST_OBJECT rather than LIST_RESOURCE:
 * MerchantCategory, MerchantCountry and CardMethod are each real sdk-python
 * resources with their own query(), but none is registered in this build, so
 * there is no same-named table for a LIST_RESOURCE ref to point at. Modelling
 * them as opaque objects costs nothing today - a caller sends
 * [{"code": "fastFoodRestaurants"}] exactly as the docs show - and promoting
 * them to LIST_RESOURCE is a mechanical follow-up once those three resources
 * exist here.
 *
 * There is no PATCH bit anywhere in this table: CorporateRule has no update()
 * in sdk-python. A caller changes a holder's or card's rules by resending the
 * whole "rules" list on THAT resource's own update() call, which is why
 * CorporateHolder.rules and CorporateCard.rules carry PATCH and none of
 * CorporateRule's own fields do.
 */

#ifndef STARKBANK_CORPORATERULE_H
#define STARKBANK_CORPORATERULE_H

#define STARKBANK_CORPORATE_RULE_FIELDS(F)                                          \
/*    wire key           type               ref     flags                       */ \
    F("name",            STRING,            NULL,  REQUIRED | CREATE)               \
    F("amount",          AMOUNT,            NULL,  REQUIRED | CREATE)               \
    F("interval",        STRING,            NULL,  CREATE)                          \
    F("schedule",        STRING,            NULL,  CREATE)                          \
    F("purposes",        LIST_STRING,       NULL,  CREATE)                          \
    F("currencyCode",    STRING,            NULL,  CREATE)                          \
    F("categories",      LIST_OBJECT,       NULL,  CREATE)                          \
    F("countries",       LIST_OBJECT,       NULL,  CREATE)                          \
    F("methods",         LIST_OBJECT,       NULL,  CREATE)                          \
    F("id",              STRING,            NULL,  RO)                              \
    F("counterAmount",   AMOUNT,            NULL,  RO)                              \
    F("currencySymbol",  STRING,            NULL,  RO)                              \
    F("currencyName",    STRING,            NULL,  RO)

#endif /* STARKBANK_CORPORATERULE_H */
