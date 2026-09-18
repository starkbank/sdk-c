/* No pdf verb: sdk-python's merchantpurchase.Log has none. */

#include "../../starkc/verbs.h"

#include "merchantpurchase.h"

static const char *const merchantPurchaseLogQuery[] = {
    "limit", "after", "before", "types", "purchaseIds", NULL
};

STARKBANK_RESOURCE(merchant_purchase_log, "MerchantPurchaseLog",
                   STARKBANK_MERCHANT_PURCHASE_LOG_FIELDS, merchantPurchaseLogQuery);

STARKBANK_VERB_PARAMS(merchant_purchase_log)
STARKBANK_VERB_GET_ID(merchant_purchase_log)
STARKBANK_VERB_QUERY(merchant_purchase_log)
STARKBANK_VERB_PAGE(merchant_purchase_log)
