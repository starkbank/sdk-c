#include "../../starkc/verbs.h"

#include "invoice.h"

static const char *const invoiceQuery[] = {
    "limit", "after", "before", "status", "tags", "ids", NULL
};

STARKBANK_RESOURCE(invoice, "Invoice", STARKBANK_INVOICE_FIELDS, invoiceQuery);

STARKBANK_VERB_NEW(invoice)
STARKBANK_VERB_PARAMS(invoice)
STARKBANK_VERB_POST_MULTI(invoice)
STARKBANK_VERB_GET_ID(invoice)
STARKBANK_VERB_QUERY(invoice)
STARKBANK_VERB_PAGE(invoice)
STARKBANK_VERB_PATCH_ID(invoice)
STARKBANK_VERB_CONTENT(invoice, pdf)
STARKBANK_VERB_CONTENT_INT(invoice, qrcode, "size", 1, 50)
STARKBANK_VERB_SUB_RESOURCE(invoice, payment, "Payment", "Invoice.Payment")
