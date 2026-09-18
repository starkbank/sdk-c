#include "../../starkc/verbs.h"

#include "merchantcategory.h"

/* No "limit": sdk-python's query() does not take one. See merchantcategory.h. */
static const char *const merchantCategoryQuery[] = { "search", NULL };

STARKBANK_RESOURCE(merchant_category, "MerchantCategory", STARKBANK_MERCHANT_CATEGORY_FIELDS,
                   merchantCategoryQuery);

STARKBANK_VERB_PARAMS(merchant_category)
STARKBANK_VERB_QUERY(merchant_category)
