#include "../../starkc/verbs.h"

#include "brcodepayment.h"

STARKBANK_RESOURCE(brcode_payment_rule, "BrcodePayment.Rule",
                   STARKBANK_BRCODEPAYMENT_RULE_FIELDS, NULL);

STARKBANK_VERB_NEW(brcode_payment_rule)
