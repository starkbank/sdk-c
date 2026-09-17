#include "../../starkc/verbs.h"

#include "deposit.h"

static const char *const depositQuery[] = {
    "limit", "after", "before", "status", "sort", "tags", "ids", NULL
};

STARKBANK_RESOURCE(deposit, "Deposit", STARKBANK_DEPOSIT_FIELDS, depositQuery);

STARKBANK_VERB_PARAMS(deposit)
STARKBANK_VERB_GET_ID(deposit)
STARKBANK_VERB_QUERY(deposit)
STARKBANK_VERB_PAGE(deposit)
STARKBANK_VERB_PATCH_ID(deposit)
