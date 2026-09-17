#include "../../starkc/verbs.h"

#include "transaction.h"

static const char *const transactionQuery[] = {
    "limit", "after", "before", "tags", "externalIds", "ids", NULL
};

STARKBANK_RESOURCE(transaction, "Transaction", STARKBANK_TRANSACTION_FIELDS, transactionQuery);

STARKBANK_VERB_PARAMS(transaction)
STARKBANK_VERB_GET_ID(transaction)
STARKBANK_VERB_QUERY(transaction)
STARKBANK_VERB_PAGE(transaction)
