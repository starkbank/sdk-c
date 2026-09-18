/* No pdf verb: sdk-python's split.Log has none. errors is LIST_STRING - see
   split.h. */

#include "../../starkc/verbs.h"

#include "split.h"

static const char *const splitLogQuery[] = {
    "limit", "after", "before", "types", "splitIds", NULL
};

STARKBANK_RESOURCE(split_log, "SplitLog", STARKBANK_SPLIT_LOG_FIELDS, splitLogQuery);

STARKBANK_VERB_PARAMS(split_log)
STARKBANK_VERB_GET_ID(split_log)
STARKBANK_VERB_QUERY(split_log)
STARKBANK_VERB_PAGE(split_log)
