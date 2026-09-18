/* No create/update/delete: every field is RO, generated automatically when a
   MerchantPurchase is split. See merchantinstallment.h. */

#include "../../starkc/verbs.h"

#include "merchantinstallment.h"

static const char *const merchantInstallmentQuery[] = {
    "limit", "after", "before", "status", "tags", "ids", "purchaseIds", NULL
};

STARKBANK_RESOURCE(merchant_installment, "MerchantInstallment",
                   STARKBANK_MERCHANT_INSTALLMENT_FIELDS, merchantInstallmentQuery);

STARKBANK_VERB_PARAMS(merchant_installment)
STARKBANK_VERB_GET_ID(merchant_installment)
STARKBANK_VERB_QUERY(merchant_installment)
STARKBANK_VERB_PAGE(merchant_installment)
