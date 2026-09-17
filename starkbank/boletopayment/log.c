/* "BoletoPaymentLog" becomes /v2/boleto-payment/log at run time, in
   starkcore_api_endpoint. No pdf verb: sdk-python's boletopayment.log has
   none, and python is normative for the verb surface. The receipt is
   starkbank_boleto_payment_pdf. */

#include "../../starkc/verbs.h"

#include "boletopayment.h"

static const char *const boletoPaymentLogQuery[] = {
    "limit", "after", "before", "types", "paymentIds", NULL
};

STARKBANK_RESOURCE(boleto_payment_log, "BoletoPaymentLog", STARKBANK_BOLETOPAYMENT_LOG_FIELDS,
                   boletoPaymentLogQuery);

STARKBANK_VERB_PARAMS(boleto_payment_log)
STARKBANK_VERB_GET_ID(boleto_payment_log)
STARKBANK_VERB_QUERY(boleto_payment_log)
STARKBANK_VERB_PAGE(boleto_payment_log)
