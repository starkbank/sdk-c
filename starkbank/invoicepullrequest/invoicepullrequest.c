/* sdk-python spells the DELETE /v2/invoice-pull-request/:id call cancel();
   sdk-c keeps the uniform starkbank_invoice_pull_request_delete spelling, as
   for InvoicePullSubscription. */

#include "../../starkc/verbs.h"

#include "invoicepullrequest.h"

static const char *const invoicePullRequestQuery[] = {
    "limit", "after", "before", "status", "invoiceIds", "subscriptionIds",
    "externalIds", "tags", "ids", NULL
};

STARKBANK_RESOURCE(invoice_pull_request, "InvoicePullRequest",
                   STARKBANK_INVOICE_PULL_REQUEST_FIELDS, invoicePullRequestQuery);

STARKBANK_VERB_NEW(invoice_pull_request)
STARKBANK_VERB_PARAMS(invoice_pull_request)
STARKBANK_VERB_POST_MULTI(invoice_pull_request)
STARKBANK_VERB_GET_ID(invoice_pull_request)
STARKBANK_VERB_QUERY(invoice_pull_request)
STARKBANK_VERB_PAGE(invoice_pull_request)
STARKBANK_VERB_DELETE_ID(invoice_pull_request)
