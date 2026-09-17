/* "BoletoLog" becomes /v2/boleto/log at run time, in starkcore_api_endpoint.
   No pdf verb: sdk-python's boleto.log has none, and python is normative for
   the verb surface. The receipt is starkbank_boleto_pdf. */

#include "../../starkc/verbs.h"

#include "boleto.h"

static const char *const boletoLogQuery[] = {
    "limit", "after", "before", "types", "boletoIds", NULL
};

STARKBANK_RESOURCE(boleto_log, "BoletoLog", STARKBANK_BOLETO_LOG_FIELDS, boletoLogQuery);

STARKBANK_VERB_PARAMS(boleto_log)
STARKBANK_VERB_GET_ID(boleto_log)
STARKBANK_VERB_QUERY(boleto_log)
STARKBANK_VERB_PAGE(boleto_log)
