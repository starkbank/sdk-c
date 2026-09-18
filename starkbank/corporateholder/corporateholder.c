#include "../../starkc/verbs.h"

#include "corporateholder.h"

static const char *const corporateHolderQuery[] = {
    "limit", "after", "before", "ids", "status", "tags", "expand", NULL
};

STARKBANK_RESOURCE(corporate_holder, "CorporateHolder", STARKBANK_CORPORATE_HOLDER_FIELDS,
                   corporateHolderQuery);

STARKBANK_VERB_NEW(corporate_holder)
STARKBANK_VERB_PARAMS(corporate_holder)
STARKBANK_VERB_POST_MULTI(corporate_holder)
STARKBANK_VERB_GET_ID(corporate_holder)
STARKBANK_VERB_QUERY(corporate_holder)
STARKBANK_VERB_PAGE(corporate_holder)
STARKBANK_VERB_PATCH_ID(corporate_holder)
STARKBANK_VERB_DELETE_ID(corporate_holder)
