/* No pdf verb: sdk-python's verifiedaccount.Log has none. errors is
   LIST_OBJECT, not LIST_STRING - see verifiedaccount.h. */

#include "../../starkc/verbs.h"

#include "verifiedaccount.h"

static const char *const verifiedAccountLogQuery[] = {
    "limit", "after", "before", "types", "accountIds", NULL
};

STARKBANK_RESOURCE(verified_account_log, "VerifiedAccountLog",
                   STARKBANK_VERIFIED_ACCOUNT_LOG_FIELDS, verifiedAccountLogQuery);

STARKBANK_VERB_PARAMS(verified_account_log)
STARKBANK_VERB_GET_ID(verified_account_log)
STARKBANK_VERB_QUERY(verified_account_log)
STARKBANK_VERB_PAGE(verified_account_log)
