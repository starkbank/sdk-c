/* "UtilityPaymentLog" becomes /v2/utility-payment/log at run time, in
   starkcore_api_endpoint. No pdf verb: sdk-python's utilitypayment.log has
   none, and python is normative for the verb surface. The receipt is
   starkbank_utility_payment_pdf. */

#include "../../starkc/verbs.h"

#include "utilitypayment.h"

static const char *const utilityPaymentLogQuery[] = {
    "limit", "after", "before", "types", "paymentIds", NULL
};

STARKBANK_RESOURCE(utility_payment_log, "UtilityPaymentLog", STARKBANK_UTILITYPAYMENT_LOG_FIELDS,
                   utilityPaymentLogQuery);

STARKBANK_VERB_PARAMS(utility_payment_log)
STARKBANK_VERB_GET_ID(utility_payment_log)
STARKBANK_VERB_QUERY(utility_payment_log)
STARKBANK_VERB_PAGE(utility_payment_log)
