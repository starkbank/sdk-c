#include "../../starkc/verbs.h"

#include "brcodepayment.h"

static const char *const brcodePaymentQuery[] = {
    "limit", "after", "before", "tags", "ids", "status", NULL
};

STARKBANK_RESOURCE(brcode_payment, "BrcodePayment", STARKBANK_BRCODEPAYMENT_FIELDS,
                   brcodePaymentQuery);

STARKBANK_VERB_NEW(brcode_payment)
STARKBANK_VERB_PARAMS(brcode_payment)
STARKBANK_VERB_POST_MULTI(brcode_payment)
STARKBANK_VERB_GET_ID(brcode_payment)
STARKBANK_VERB_QUERY(brcode_payment)
STARKBANK_VERB_PAGE(brcode_payment)
STARKBANK_VERB_PATCH_ID(brcode_payment)
STARKBANK_VERB_CONTENT(brcode_payment, pdf)
