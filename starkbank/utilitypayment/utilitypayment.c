#include "../../starkc/verbs.h"

#include "utilitypayment.h"

static const char *const utilityPaymentQuery[] = {
    "limit", "after", "before", "tags", "ids", "status", NULL
};

STARKBANK_RESOURCE(utility_payment, "UtilityPayment", STARKBANK_UTILITYPAYMENT_FIELDS,
                   utilityPaymentQuery);

STARKBANK_VERB_NEW(utility_payment)
STARKBANK_VERB_PARAMS(utility_payment)
STARKBANK_VERB_POST_MULTI(utility_payment)
STARKBANK_VERB_GET_ID(utility_payment)
STARKBANK_VERB_DELETE_ID(utility_payment)
STARKBANK_VERB_QUERY(utility_payment)
STARKBANK_VERB_PAGE(utility_payment)
STARKBANK_VERB_CONTENT(utility_payment, pdf)
