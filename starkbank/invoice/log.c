/* "InvoiceLog" becomes /v2/invoice/log at run time, in starkcore_api_endpoint. */

#include "../../starkc/verbs.h"

#include "invoice.h"

static const char *const invoiceLogQuery[] = {
    "limit", "after", "before", "types", "invoiceIds", NULL
};

STARKBANK_RESOURCE(invoice_log, "InvoiceLog", STARKBANK_INVOICE_LOG_FIELDS, invoiceLogQuery);

STARKBANK_VERB_PARAMS(invoice_log)
STARKBANK_VERB_GET_ID(invoice_log)
STARKBANK_VERB_QUERY(invoice_log)
STARKBANK_VERB_PAGE(invoice_log)
STARKBANK_VERB_CONTENT(invoice_log, pdf)
