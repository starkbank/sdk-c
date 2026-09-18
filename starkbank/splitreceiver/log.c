/* No pdf verb: sdk-python's splitreceiver.Log has none. errors is
   LIST_STRING - see splitreceiver.h. */

#include "../../starkc/verbs.h"

#include "splitreceiver.h"

static const char *const splitReceiverLogQuery[] = {
    "limit", "after", "before", "types", "receiverIds", NULL
};

STARKBANK_RESOURCE(split_receiver_log, "SplitReceiverLog", STARKBANK_SPLIT_RECEIVER_LOG_FIELDS,
                   splitReceiverLogQuery);

STARKBANK_VERB_PARAMS(split_receiver_log)
STARKBANK_VERB_GET_ID(split_receiver_log)
STARKBANK_VERB_QUERY(split_receiver_log)
STARKBANK_VERB_PAGE(split_receiver_log)
