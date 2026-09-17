/* No pdf verb: sdk-python's corporatepurchase.Log has none. */

#include "../../starkc/verbs.h"

#include "corporatepurchase.h"

static const char *const corporatePurchaseLogQuery[] = {
    "ids", "limit", "after", "before", "types", "purchaseIds", NULL
};

STARKBANK_RESOURCE(corporate_purchase_log, "CorporatePurchaseLog",
                   STARKBANK_CORPORATE_PURCHASE_LOG_FIELDS, corporatePurchaseLogQuery);

STARKBANK_VERB_PARAMS(corporate_purchase_log)
STARKBANK_VERB_GET_ID(corporate_purchase_log)
STARKBANK_VERB_QUERY(corporate_purchase_log)
STARKBANK_VERB_PAGE(corporate_purchase_log)
