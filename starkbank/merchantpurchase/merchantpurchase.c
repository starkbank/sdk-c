#include "../../starkc/verbs.h"

#include "merchantpurchase.h"

static const char *const merchantPurchaseQuery[] = {
    "limit", "after", "before", "status", "tags", "ids", "holderId", NULL
};

STARKBANK_RESOURCE(merchant_purchase, "MerchantPurchase", STARKBANK_MERCHANT_PURCHASE_FIELDS,
                   merchantPurchaseQuery);

STARKBANK_VERB_NEW(merchant_purchase)
STARKBANK_VERB_PARAMS(merchant_purchase)
STARKBANK_VERB_POST_SINGLE(merchant_purchase)
STARKBANK_VERB_GET_ID(merchant_purchase)
STARKBANK_VERB_QUERY(merchant_purchase)
STARKBANK_VERB_PAGE(merchant_purchase)
STARKBANK_VERB_PATCH_ID(merchant_purchase)
