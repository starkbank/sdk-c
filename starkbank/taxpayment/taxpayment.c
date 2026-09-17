#include "../../starkc/verbs.h"

#include "taxpayment.h"

static const char *const taxPaymentQuery[] = {
    "limit", "after", "before", "tags", "ids", "status", NULL
};

STARKBANK_RESOURCE(tax_payment, "TaxPayment", STARKBANK_TAXPAYMENT_FIELDS,
                   taxPaymentQuery);

STARKBANK_VERB_NEW(tax_payment)
STARKBANK_VERB_PARAMS(tax_payment)
STARKBANK_VERB_POST_MULTI(tax_payment)
STARKBANK_VERB_GET_ID(tax_payment)
STARKBANK_VERB_DELETE_ID(tax_payment)
STARKBANK_VERB_QUERY(tax_payment)
STARKBANK_VERB_PAGE(tax_payment)
STARKBANK_VERB_CONTENT(tax_payment, pdf)
