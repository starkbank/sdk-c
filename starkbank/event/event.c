#include <string.h>

#include "../../starkc/verbs.h"

#include "event.h"

static const char *const eventQuery[] = { "limit", "after", "before", "isDelivered", NULL };

/*
 * sdk-python's _resource_by_subscription, verbatim and in its order. Eight of
 * these ten tables arrive with the rest of the bank surface; until then
 * starkbankRegistryFind returns NULL for them and the log hydrates untagged
 * and permissive, with the unknown counter reporting the gap. That is the same
 * path a subscription invented after this build ships will take, so the case
 * is exercised on every run rather than waiting for the API to grow one.
 */
static const char *const subscriptionTables[][2] = {
    { "transfer",        "TransferLog" },
    { "invoice",         "InvoiceLog" },
    { "deposit",         "DepositLog" },
    { "boleto",          "BoletoLog" },
    { "brcode-payment",  "BrcodePaymentLog" },
    { "boleto-payment",  "BoletoPaymentLog" },
    { "utility-payment", "UtilityPaymentLog" },
    { "darf-payment",    "DarfPaymentLog" },
    { "tax-payment",     "TaxPaymentLog" },
    { "holmes",          "BoletoHolmesLog" }
};

static const starkbankResource * eventLogResource(const starkcore_json *object,
                                                  const char *field)
{
    const starkcore_json *subscription;
    const char *value;
    size_t index;

    if (strcmp(field, "log") != 0) {
        return NULL;
    }
    subscription = starkcore_json_get(object, "subscription");
    value = subscription != NULL ? starkcore_json_string(subscription) : NULL;
    if (value == NULL) {
        return NULL;
    }
    for (index = 0; index < sizeof(subscriptionTables) / sizeof(subscriptionTables[0]); index++) {
        if (strcmp(subscriptionTables[index][0], value) == 0) {
            return starkbankRegistryFind(subscriptionTables[index][1]);
        }
    }
    return NULL;
}

STARKBANK_RESOURCE_FULL(event, "Event", STARKBANK_EVENT_FIELDS, eventQuery, eventLogResource);

STARKBANK_VERB_PARAMS(event)
STARKBANK_VERB_GET_ID(event)
STARKBANK_VERB_QUERY(event)
STARKBANK_VERB_PAGE(event)
STARKBANK_VERB_PATCH_ID(event)
STARKBANK_VERB_DELETE_ID(event)

/* starkbank_event_parse is declared in the public header and defined by hand
   in handwritten/event_parse.c. No macro here will ever define it: if that
   file goes away the link fails, which is the only "this needs a human"
   marker that cannot be ignored. */
