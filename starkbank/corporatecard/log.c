/* No pdf verb: sdk-python's corporatecard.Log has none. */

#include "../../starkc/verbs.h"

#include "corporatecard.h"

static const char *const corporateCardLogQuery[] = {
    "limit", "after", "before", "types", "cardIds", "ids", NULL
};

STARKBANK_RESOURCE(corporate_card_log, "CorporateCardLog", STARKBANK_CORPORATE_CARD_LOG_FIELDS,
                   corporateCardLogQuery);

STARKBANK_VERB_PARAMS(corporate_card_log)
STARKBANK_VERB_GET_ID(corporate_card_log)
STARKBANK_VERB_QUERY(corporate_card_log)
STARKBANK_VERB_PAGE(corporate_card_log)
