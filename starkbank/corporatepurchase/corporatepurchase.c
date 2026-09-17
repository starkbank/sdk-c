#include "../../starkc/verbs.h"

#include "corporatepurchase.h"

static const char *const corporatePurchaseQuery[] = {
    "ids", "limit", "after", "before", "merchantCategoryTypes", "holderIds",
    "cardIds", "status", NULL
};

STARKBANK_RESOURCE(corporate_purchase, "CorporatePurchase", STARKBANK_CORPORATE_PURCHASE_FIELDS,
                   corporatePurchaseQuery);

STARKBANK_VERB_PARAMS(corporate_purchase)
STARKBANK_VERB_GET_ID(corporate_purchase)
STARKBANK_VERB_QUERY(corporate_purchase)
STARKBANK_VERB_PAGE(corporate_purchase)

/* starkbank_corporate_purchase_parse and _response are declared in the public
   header and defined by hand in handwritten/. No macro here will ever define
   them: neither is a REST verb. */
