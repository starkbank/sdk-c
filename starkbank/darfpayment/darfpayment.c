#include "../../starkc/verbs.h"

#include "darfpayment.h"

static const char *const darfPaymentQuery[] = {
    "limit", "after", "before", "tags", "ids", "status", NULL
};

STARKBANK_RESOURCE(darf_payment, "DarfPayment", STARKBANK_DARFPAYMENT_FIELDS,
                   darfPaymentQuery);

STARKBANK_VERB_NEW(darf_payment)
STARKBANK_VERB_PARAMS(darf_payment)
STARKBANK_VERB_POST_MULTI(darf_payment)
STARKBANK_VERB_GET_ID(darf_payment)
STARKBANK_VERB_DELETE_ID(darf_payment)
STARKBANK_VERB_QUERY(darf_payment)
STARKBANK_VERB_PAGE(darf_payment)
STARKBANK_VERB_CONTENT(darf_payment, pdf)
