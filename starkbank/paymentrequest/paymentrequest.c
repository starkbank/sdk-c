/* No get(): sdk-python's paymentrequest module has none, only create, query
   and page - see paymentrequest.h. */

#include "../../starkc/verbs.h"

#include "paymentrequest.h"

static const char *const paymentRequestQuery[] = {
    "centerId", "limit", "after", "before", "sort", "status", "type", "tags", "ids", NULL
};

STARKBANK_POLYMORPH(payment_request, "payment", "type", STARKBANK_PAYMENT_REQUEST_VARIANTS);

STARKBANK_RESOURCE_FULL(payment_request, "PaymentRequest", STARKBANK_PAYMENT_REQUEST_FIELDS,
                        paymentRequestQuery, starkbankPolymorph_payment_request);

STARKBANK_VERB_NEW(payment_request)
STARKBANK_VERB_PARAMS(payment_request)
STARKBANK_VERB_POST_MULTI(payment_request)
STARKBANK_VERB_QUERY(payment_request)
STARKBANK_VERB_PAGE(payment_request)
