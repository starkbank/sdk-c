#include "../../starkc/verbs.h"

#include "boletoholmes.h"

static const char *const boletoHolmesQuery[] = {
    "limit", "after", "before", "tags", "ids", "status", "boletoId", NULL
};

STARKBANK_RESOURCE(boleto_holmes, "BoletoHolmes", STARKBANK_BOLETO_HOLMES_FIELDS,
                   boletoHolmesQuery);

STARKBANK_VERB_NEW(boleto_holmes)
STARKBANK_VERB_PARAMS(boleto_holmes)
STARKBANK_VERB_POST_MULTI(boleto_holmes)
STARKBANK_VERB_GET_ID(boleto_holmes)
STARKBANK_VERB_QUERY(boleto_holmes)
STARKBANK_VERB_PAGE(boleto_holmes)
