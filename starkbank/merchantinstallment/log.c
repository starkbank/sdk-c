/* No pdf verb: sdk-python's merchantinstallment.Log has none. */

#include "../../starkc/verbs.h"

#include "merchantinstallment.h"

static const char *const merchantInstallmentLogQuery[] = {
    "limit", "after", "before", "types", "installmentIds", NULL
};

STARKBANK_RESOURCE(merchant_installment_log, "MerchantInstallmentLog",
                   STARKBANK_MERCHANT_INSTALLMENT_LOG_FIELDS, merchantInstallmentLogQuery);

STARKBANK_VERB_PARAMS(merchant_installment_log)
STARKBANK_VERB_GET_ID(merchant_installment_log)
STARKBANK_VERB_QUERY(merchant_installment_log)
STARKBANK_VERB_PAGE(merchant_installment_log)
