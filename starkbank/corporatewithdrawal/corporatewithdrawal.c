#include "../../starkc/verbs.h"

#include "corporatewithdrawal.h"

static const char *const corporateWithdrawalQuery[] = {
    "limit", "after", "before", "tags", "externalIds", NULL
};

STARKBANK_RESOURCE(corporate_withdrawal, "CorporateWithdrawal",
                   STARKBANK_CORPORATE_WITHDRAWAL_FIELDS, corporateWithdrawalQuery);

STARKBANK_VERB_NEW(corporate_withdrawal)
STARKBANK_VERB_PARAMS(corporate_withdrawal)
STARKBANK_VERB_POST_SINGLE(corporate_withdrawal)
STARKBANK_VERB_GET_ID(corporate_withdrawal)
STARKBANK_VERB_QUERY(corporate_withdrawal)
STARKBANK_VERB_PAGE(corporate_withdrawal)
