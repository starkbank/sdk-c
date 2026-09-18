/* Sub-resource, no verbs of its own: embedded only in
   MerchantSession.allowedInstallments. Registered as
   "MerchantSession.AllowedInstallment", not bare "AllowedInstallment" - see
   merchantsession.h for why sdk-python's _sub_resource dict forces the
   qualified name here even though nothing else in the SDK shares it. */

#include "../../starkc/verbs.h"

#include "merchantsession.h"

STARKBANK_RESOURCE(allowed_installment, "MerchantSession.AllowedInstallment",
                   STARKBANK_ALLOWED_INSTALLMENT_FIELDS, NULL);

STARKBANK_VERB_NEW(allowed_installment)
