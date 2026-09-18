#include "../../starkc/verbs.h"

#include "verifiedaccount.h"

static const char *const verifiedAccountQuery[] = {
    "limit", "after", "before", "status", "ids", "tags", NULL
};

STARKBANK_RESOURCE(verified_account, "VerifiedAccount", STARKBANK_VERIFIED_ACCOUNT_FIELDS,
                   verifiedAccountQuery);

STARKBANK_VERB_NEW(verified_account)
STARKBANK_VERB_PARAMS(verified_account)
STARKBANK_VERB_POST_MULTI(verified_account)
STARKBANK_VERB_GET_ID(verified_account)
STARKBANK_VERB_DELETE_ID(verified_account)
STARKBANK_VERB_QUERY(verified_account)
STARKBANK_VERB_PAGE(verified_account)
