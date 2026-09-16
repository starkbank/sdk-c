#include "../../starkc/verbs.h"

#include "invoice.h"

STARKBANK_RESOURCE(invoice_rule, "Invoice.Rule", STARKBANK_INVOICE_RULE_FIELDS, NULL);

STARKBANK_VERB_NEW(invoice_rule)
