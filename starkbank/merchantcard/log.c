/* No pdf verb: sdk-python's merchantcard.Log has none. */

#include "../../starkc/verbs.h"

#include "merchantcard.h"

static const char *const merchantCardLogQuery[] = {
    "limit", "cardIds", "after", "before", "types", NULL
};

STARKBANK_RESOURCE(merchant_card_log, "MerchantCardLog", STARKBANK_MERCHANT_CARD_LOG_FIELDS,
                   merchantCardLogQuery);

STARKBANK_VERB_PARAMS(merchant_card_log)
STARKBANK_VERB_GET_ID(merchant_card_log)
STARKBANK_VERB_QUERY(merchant_card_log)
STARKBANK_VERB_PAGE(merchant_card_log)
