#include "../../starkc/verbs.h"

#include "merchantcountry.h"

/* No "limit": sdk-python's query() does not take one. See merchantcountry.h. */
static const char *const merchantCountryQuery[] = { "search", NULL };

STARKBANK_RESOURCE(merchant_country, "MerchantCountry", STARKBANK_MERCHANT_COUNTRY_FIELDS,
                   merchantCountryQuery);

STARKBANK_VERB_PARAMS(merchant_country)
STARKBANK_VERB_QUERY(merchant_country)
