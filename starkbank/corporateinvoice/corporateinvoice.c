#include "../../starkc/verbs.h"

#include "corporateinvoice.h"

static const char *const corporateInvoiceQuery[] = {
    "limit", "after", "before", "status", "tags", NULL
};

STARKBANK_RESOURCE(corporate_invoice, "CorporateInvoice", STARKBANK_CORPORATE_INVOICE_FIELDS,
                   corporateInvoiceQuery);

STARKBANK_VERB_NEW(corporate_invoice)
STARKBANK_VERB_PARAMS(corporate_invoice)
STARKBANK_VERB_POST_SINGLE(corporate_invoice)
STARKBANK_VERB_QUERY(corporate_invoice)
STARKBANK_VERB_PAGE(corporate_invoice)
