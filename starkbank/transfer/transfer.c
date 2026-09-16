#include "../../starkc/verbs.h"

#include "transfer.h"

static const char *const transferQuery[] = {
    "limit", "after", "before", "transactionIds", "status", "taxId", "sort", "tags", "ids", NULL
};

STARKBANK_RESOURCE(transfer, "Transfer", STARKBANK_TRANSFER_FIELDS, transferQuery);

STARKBANK_VERB_NEW(transfer)
STARKBANK_VERB_PARAMS(transfer)
STARKBANK_VERB_POST_MULTI(transfer)
STARKBANK_VERB_GET_ID(transfer)
STARKBANK_VERB_DELETE_ID(transfer)
STARKBANK_VERB_QUERY(transfer)
STARKBANK_VERB_PAGE(transfer)
STARKBANK_VERB_CONTENT(transfer, pdf)
