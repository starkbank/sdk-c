/* sdk-python spells the DELETE /v2/invoice-pull-subscription/:id call
   cancel(); sdk-c keeps every resource's DELETE_ID verb named
   starkbank_<resource>_delete for one consistent spelling, the same choice
   documented at tests/reference/known-drift.json's
   verb.new:CorporateHolder:cancel. */

#include "../../starkc/verbs.h"

#include "invoicepullsubscription.h"

static const char *const invoicePullSubscriptionQuery[] = {
    "limit", "after", "before", "status", "invoiceIds", "externalIds", "tags", "ids", NULL
};

STARKBANK_RESOURCE(invoice_pull_subscription, "InvoicePullSubscription",
                   STARKBANK_INVOICE_PULL_SUBSCRIPTION_FIELDS, invoicePullSubscriptionQuery);

STARKBANK_VERB_NEW(invoice_pull_subscription)
STARKBANK_VERB_PARAMS(invoice_pull_subscription)
STARKBANK_VERB_POST_MULTI(invoice_pull_subscription)
STARKBANK_VERB_GET_ID(invoice_pull_subscription)
STARKBANK_VERB_QUERY(invoice_pull_subscription)
STARKBANK_VERB_PAGE(invoice_pull_subscription)
STARKBANK_VERB_DELETE_ID(invoice_pull_subscription)
