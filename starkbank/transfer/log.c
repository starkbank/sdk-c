/* No pdf verb: sdk-python's transfer.Log has none, and python is normative
   for the verb surface. The receipt is starkbank_transfer_pdf. */

#include "../../starkc/verbs.h"

#include "transfer.h"

static const char *const transferLogQuery[] = {
    "limit", "after", "before", "types", "transferIds", NULL
};

STARKBANK_RESOURCE(transfer_log, "TransferLog", STARKBANK_TRANSFER_LOG_FIELDS, transferLogQuery);

STARKBANK_VERB_PARAMS(transfer_log)
STARKBANK_VERB_GET_ID(transfer_log)
STARKBANK_VERB_QUERY(transfer_log)
STARKBANK_VERB_PAGE(transfer_log)
