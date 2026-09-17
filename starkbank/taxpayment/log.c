/* "TaxPaymentLog" becomes /v2/tax-payment/log at run time, in
   starkcore_api_endpoint. No pdf verb: sdk-python's taxpayment.log has none,
   and python is normative for the verb surface. The receipt is
   starkbank_tax_payment_pdf. */

#include "../../starkc/verbs.h"

#include "taxpayment.h"

static const char *const taxPaymentLogQuery[] = {
    "limit", "after", "before", "types", "paymentIds", NULL
};

STARKBANK_RESOURCE(tax_payment_log, "TaxPaymentLog", STARKBANK_TAXPAYMENT_LOG_FIELDS,
                   taxPaymentLogQuery);

STARKBANK_VERB_PARAMS(tax_payment_log)
STARKBANK_VERB_GET_ID(tax_payment_log)
STARKBANK_VERB_QUERY(tax_payment_log)
STARKBANK_VERB_PAGE(tax_payment_log)
