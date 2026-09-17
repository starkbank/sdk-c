/* "DepositLog" becomes /v2/deposit/log at run time, in starkcore_api_endpoint.
   deposit.Log has a pdf verb - the reversed deposit's receipt - where
   transfer.Log, boleto.Log, boletopayment.Log and brcodepayment.Log have
   none: each log's pdf follows its own python module, never a blanket rule. */

#include "../../starkc/verbs.h"

#include "deposit.h"

static const char *const depositLogQuery[] = {
    "limit", "after", "before", "types", "depositIds", NULL
};

STARKBANK_RESOURCE(deposit_log, "DepositLog", STARKBANK_DEPOSIT_LOG_FIELDS, depositLogQuery);

STARKBANK_VERB_PARAMS(deposit_log)
STARKBANK_VERB_GET_ID(deposit_log)
STARKBANK_VERB_QUERY(deposit_log)
STARKBANK_VERB_PAGE(deposit_log)
STARKBANK_VERB_CONTENT(deposit_log, pdf)
