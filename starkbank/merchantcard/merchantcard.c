/* No create/update/delete: every field is RO, and sdk-python's module
   exports get/query/page only. See merchantcard.h. */

#include "../../starkc/verbs.h"

#include "merchantcard.h"

static const char *const merchantCardQuery[] = {
    "limit", "after", "before", "status", "tags", "ids", NULL
};

STARKBANK_RESOURCE(merchant_card, "MerchantCard", STARKBANK_MERCHANT_CARD_FIELDS,
                   merchantCardQuery);

STARKBANK_VERB_PARAMS(merchant_card)
STARKBANK_VERB_GET_ID(merchant_card)
STARKBANK_VERB_QUERY(merchant_card)
STARKBANK_VERB_PAGE(merchant_card)
