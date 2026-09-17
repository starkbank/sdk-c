/* "DarfPaymentLog" becomes /v2/darf-payment/log at run time, in
   starkcore_api_endpoint. No pdf verb: sdk-python's darfpayment.log has none,
   and python is normative for the verb surface. The receipt is
   starkbank_darf_payment_pdf. */

#include "../../starkc/verbs.h"

#include "darfpayment.h"

static const char *const darfPaymentLogQuery[] = {
    "limit", "after", "before", "types", "paymentIds", NULL
};

STARKBANK_RESOURCE(darf_payment_log, "DarfPaymentLog", STARKBANK_DARFPAYMENT_LOG_FIELDS,
                   darfPaymentLogQuery);

STARKBANK_VERB_PARAMS(darf_payment_log)
STARKBANK_VERB_GET_ID(darf_payment_log)
STARKBANK_VERB_QUERY(darf_payment_log)
STARKBANK_VERB_PAGE(darf_payment_log)
