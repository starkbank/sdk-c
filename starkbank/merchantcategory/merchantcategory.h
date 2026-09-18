/*
 * MerchantCategory: a query-only lookup used to build CorporateRule.categories
 * filters, e.g. [{"code": "fastFoodRestaurants"}]. Same shape as CardMethod -
 * see cardmethod.h for the "no limit, no page(), no promotion of
 * CorporateRule.categories" reasoning, which applies here unchanged.
 *
 * code and type are both filed under sdk-python's docstring as
 * "## Parameters (conditionally required)", which drift.py's checkFlags
 * treats as creatable-but-not-REQUIRED (the same bucket as "optional") -
 * exactly right here, since a caller sets exactly one of the two, never both,
 * and the API rejects neither/both rather than sdk-c refusing one locally.
 */

#ifndef STARKBANK_MERCHANTCATEGORY_H
#define STARKBANK_MERCHANTCATEGORY_H

#define STARKBANK_MERCHANT_CATEGORY_FIELDS(F)                                        \
/*    wire key           type               ref                 flags           */ \
    F("code",            STRING,            NULL,  CREATE)                          \
    F("type",            STRING,            NULL,  CREATE)                          \
    F("name",            STRING,            NULL,  RO)                              \
    F("number",          STRING,            NULL,  RO)

#endif /* STARKBANK_MERCHANTCATEGORY_H */
