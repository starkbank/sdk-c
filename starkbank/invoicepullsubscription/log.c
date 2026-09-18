/* No pdf verb: sdk-python's invoicepullsubscription.Log has none. errors is
   LIST_OBJECT, not LIST_STRING - see invoicepullsubscription.h. */

#include "../../starkc/verbs.h"

#include "invoicepullsubscription.h"

static const char *const invoicePullSubscriptionLogQuery[] = {
    "limit", "after", "before", "types", "subscriptionIds", NULL
};

STARKBANK_RESOURCE(invoice_pull_subscription_log, "InvoicePullSubscriptionLog",
                   STARKBANK_INVOICE_PULL_SUBSCRIPTION_LOG_FIELDS,
                   invoicePullSubscriptionLogQuery);

STARKBANK_VERB_PARAMS(invoice_pull_subscription_log)
STARKBANK_VERB_GET_ID(invoice_pull_subscription_log)
STARKBANK_VERB_QUERY(invoice_pull_subscription_log)
STARKBANK_VERB_PAGE(invoice_pull_subscription_log)
