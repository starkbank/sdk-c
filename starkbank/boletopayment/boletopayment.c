#include "../../starkc/verbs.h"

#include "boletopayment.h"

static const char *const boletoPaymentQuery[] = {
    "limit", "after", "before", "tags", "ids", "status", NULL
};

STARKBANK_RESOURCE(boleto_payment, "BoletoPayment", STARKBANK_BOLETOPAYMENT_FIELDS,
                   boletoPaymentQuery);

STARKBANK_VERB_NEW(boleto_payment)
STARKBANK_VERB_PARAMS(boleto_payment)
STARKBANK_VERB_POST_MULTI(boleto_payment)
STARKBANK_VERB_GET_ID(boleto_payment)
STARKBANK_VERB_DELETE_ID(boleto_payment)
STARKBANK_VERB_QUERY(boleto_payment)
STARKBANK_VERB_PAGE(boleto_payment)
STARKBANK_VERB_CONTENT(boleto_payment, pdf)
