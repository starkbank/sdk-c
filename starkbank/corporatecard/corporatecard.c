#include "../../starkc/verbs.h"

#include "corporatecard.h"

static const char *const corporateCardQuery[] = {
    "limit", "after", "before", "status", "types", "holderIds", "ids", "tags", "expand", NULL
};

STARKBANK_RESOURCE(corporate_card, "CorporateCard", STARKBANK_CORPORATE_CARD_FIELDS,
                   corporateCardQuery);

STARKBANK_VERB_NEW(corporate_card)
STARKBANK_VERB_PARAMS(corporate_card)
STARKBANK_VERB_POST_SINGLE_SUB(corporate_card, "token")
STARKBANK_VERB_GET_ID(corporate_card)
STARKBANK_VERB_QUERY(corporate_card)
STARKBANK_VERB_PAGE(corporate_card)
STARKBANK_VERB_PATCH_ID(corporate_card)
STARKBANK_VERB_DELETE_ID(corporate_card)
