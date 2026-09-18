/* No pdf verb, no errors field: sdk-python's boletoholmes.Log has neither -
   see boletoholmes.h. */

#include "../../starkc/verbs.h"

#include "boletoholmes.h"

static const char *const boletoHolmesLogQuery[] = {
    "limit", "after", "before", "types", "holmesIds", NULL
};

STARKBANK_RESOURCE(boleto_holmes_log, "BoletoHolmesLog", STARKBANK_BOLETO_HOLMES_LOG_FIELDS,
                   boletoHolmesLogQuery);

STARKBANK_VERB_PARAMS(boleto_holmes_log)
STARKBANK_VERB_GET_ID(boleto_holmes_log)
STARKBANK_VERB_QUERY(boleto_holmes_log)
STARKBANK_VERB_PAGE(boleto_holmes_log)
