/* No pdf verb: sdk-python's corporateholder.Log has none. */

#include "../../starkc/verbs.h"

#include "corporateholder.h"

static const char *const corporateHolderLogQuery[] = {
    "limit", "after", "before", "types", "holderIds", "ids", NULL
};

STARKBANK_RESOURCE(corporate_holder_log, "CorporateHolderLog",
                   STARKBANK_CORPORATE_HOLDER_LOG_FIELDS, corporateHolderLogQuery);

STARKBANK_VERB_PARAMS(corporate_holder_log)
STARKBANK_VERB_GET_ID(corporate_holder_log)
STARKBANK_VERB_QUERY(corporate_holder_log)
STARKBANK_VERB_PAGE(corporate_holder_log)
