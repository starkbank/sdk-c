#include "../../starkc/verbs.h"

#include "dynamicbrcode.h"

static const char *const dynamicBrcodeQuery[] = {
    "limit", "after", "before", "tags", "uuids", NULL
};

STARKBANK_RESOURCE(dynamic_brcode, "DynamicBrcode", STARKBANK_DYNAMIC_BRCODE_FIELDS,
                   dynamicBrcodeQuery);

STARKBANK_VERB_NEW(dynamic_brcode)
STARKBANK_VERB_PARAMS(dynamic_brcode)
STARKBANK_VERB_POST_MULTI(dynamic_brcode)
STARKBANK_VERB_GET_ID(dynamic_brcode)
STARKBANK_VERB_QUERY(dynamic_brcode)
STARKBANK_VERB_PAGE(dynamic_brcode)
