/* No pdf verb: sdk-python's invoicepullrequest.Log has none. errors is
   LIST_OBJECT, not LIST_STRING - see invoicepullrequest.h. */

#include "../../starkc/verbs.h"

#include "invoicepullrequest.h"

static const char *const invoicePullRequestLogQuery[] = {
    "limit", "after", "before", "types", "requestIds", NULL
};

STARKBANK_RESOURCE(invoice_pull_request_log, "InvoicePullRequestLog",
                   STARKBANK_INVOICE_PULL_REQUEST_LOG_FIELDS, invoicePullRequestLogQuery);

STARKBANK_VERB_PARAMS(invoice_pull_request_log)
STARKBANK_VERB_GET_ID(invoice_pull_request_log)
STARKBANK_VERB_QUERY(invoice_pull_request_log)
STARKBANK_VERB_PAGE(invoice_pull_request_log)
