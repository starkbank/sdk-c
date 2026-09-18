#include "../../starkc/verbs.h"

#include "split.h"

static const char *const splitQuery[] = {
    "limit", "after", "before", "ids", "receiverIds", "status", "tags", NULL
};

STARKBANK_RESOURCE(split, "Split", STARKBANK_SPLIT_FIELDS, splitQuery);

STARKBANK_VERB_NEW(split)
STARKBANK_VERB_PARAMS(split)
STARKBANK_VERB_GET_ID(split)
STARKBANK_VERB_QUERY(split)
STARKBANK_VERB_PAGE(split)
