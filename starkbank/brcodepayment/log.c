/* "BrcodePaymentLog" becomes /v2/brcode-payment/log at run time, in
   starkcore_api_endpoint. No pdf verb: sdk-python's brcodepayment.log has
   none, and python is normative for the verb surface. The receipt is
   starkbank_brcode_payment_pdf. */

#include "../../starkc/verbs.h"

#include "brcodepayment.h"

static const char *const brcodePaymentLogQuery[] = {
    "limit", "after", "before", "types", "paymentIds", NULL
};

STARKBANK_RESOURCE(brcode_payment_log, "BrcodePaymentLog", STARKBANK_BRCODEPAYMENT_LOG_FIELDS,
                   brcodePaymentLogQuery);

STARKBANK_VERB_PARAMS(brcode_payment_log)
STARKBANK_VERB_GET_ID(brcode_payment_log)
STARKBANK_VERB_QUERY(brcode_payment_log)
STARKBANK_VERB_PAGE(brcode_payment_log)
