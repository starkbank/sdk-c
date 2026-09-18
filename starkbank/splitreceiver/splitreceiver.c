#include "../../starkc/verbs.h"

#include "splitreceiver.h"

static const char *const splitReceiverQuery[] = {
    "limit", "after", "before", "transactionIds", "status", "taxId", "sort", "tags", "ids", NULL
};

STARKBANK_RESOURCE(split_receiver, "SplitReceiver", STARKBANK_SPLIT_RECEIVER_FIELDS,
                   splitReceiverQuery);

STARKBANK_VERB_NEW(split_receiver)
STARKBANK_VERB_PARAMS(split_receiver)
STARKBANK_VERB_POST_MULTI(split_receiver)
STARKBANK_VERB_GET_ID(split_receiver)
STARKBANK_VERB_QUERY(split_receiver)
STARKBANK_VERB_PAGE(split_receiver)
