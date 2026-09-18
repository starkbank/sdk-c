#include "../../starkc/verbs.h"

#include "corporatetransaction.h"

static const char *const corporateTransactionQuery[] = {
    "limit", "after", "before", "tags", "externalIds", "ids", "source", NULL
};

STARKBANK_RESOURCE(corporate_transaction, "CorporateTransaction",
                   STARKBANK_CORPORATE_TRANSACTION_FIELDS, corporateTransactionQuery);

STARKBANK_VERB_PARAMS(corporate_transaction)
STARKBANK_VERB_GET_ID(corporate_transaction)
STARKBANK_VERB_QUERY(corporate_transaction)
STARKBANK_VERB_PAGE(corporate_transaction)
