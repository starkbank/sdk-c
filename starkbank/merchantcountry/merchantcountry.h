/*
 * MerchantCountry: a query-only lookup used to build CorporateRule.countries
 * filters, e.g. [{"code": "BRA"}]. Same shape as CardMethod - see
 * cardmethod.h for the "no limit, no page(), no promotion of
 * CorporateRule.countries" reasoning, which applies here unchanged.
 */

#ifndef STARKBANK_MERCHANTCOUNTRY_H
#define STARKBANK_MERCHANTCOUNTRY_H

#define STARKBANK_MERCHANT_COUNTRY_FIELDS(F)                                         \
/*    wire key           type               ref                 flags           */ \
    F("code",            STRING,            NULL,  REQUIRED | CREATE)               \
    F("name",            STRING,            NULL,  RO)                              \
    F("number",          STRING,            NULL,  RO)                              \
    F("shortCode",       STRING,            NULL,  RO)

#endif /* STARKBANK_MERCHANTCOUNTRY_H */
