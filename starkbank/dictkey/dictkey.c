#include "../../starkc/verbs.h"

#include "dictkey.h"

static const char *const dictKeyQuery[] = {
    "limit", "type", "after", "before", "ids", "status", NULL
};

STARKBANK_RESOURCE(dict_key, "DictKey", STARKBANK_DICT_KEY_FIELDS, dictKeyQuery);

STARKBANK_VERB_PARAMS(dict_key)
STARKBANK_VERB_GET_ID(dict_key)
STARKBANK_VERB_QUERY(dict_key)
STARKBANK_VERB_PAGE(dict_key)
