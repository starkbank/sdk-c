/* No pdf verb: sdk-python's splitprofile.Log has none. errors is
   LIST_STRING - see splitprofile.h. */

#include "../../starkc/verbs.h"

#include "splitprofile.h"

static const char *const splitProfileLogQuery[] = {
    "limit", "after", "before", "types", "profileIds", NULL
};

STARKBANK_RESOURCE(split_profile_log, "SplitProfileLog", STARKBANK_SPLIT_PROFILE_LOG_FIELDS,
                   splitProfileLogQuery);

STARKBANK_VERB_PARAMS(split_profile_log)
STARKBANK_VERB_GET_ID(split_profile_log)
STARKBANK_VERB_QUERY(split_profile_log)
STARKBANK_VERB_PAGE(split_profile_log)
