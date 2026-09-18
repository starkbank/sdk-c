#include "../../starkc/verbs.h"

#include "splitprofile.h"

static const char *const splitProfileQuery[] = {
    "limit", "after", "before", "ids", "receiverIds", "status", "tags", NULL
};

STARKBANK_RESOURCE(split_profile, "SplitProfile", STARKBANK_SPLIT_PROFILE_FIELDS,
                   splitProfileQuery);

STARKBANK_VERB_NEW(split_profile)
STARKBANK_VERB_PARAMS(split_profile)
STARKBANK_VERB_PUT_MULTI(split_profile)
STARKBANK_VERB_GET_ID(split_profile)
STARKBANK_VERB_QUERY(split_profile)
STARKBANK_VERB_PAGE(split_profile)
