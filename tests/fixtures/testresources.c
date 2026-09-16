/*
 * Authored exactly as starkbank/invoice/invoice.c will be in step 3. If this
 * file needs anything the macros do not offer, the engine is wrong.
 */

#define STARKBANK_TEST_RESOURCE_PROTOTYPES

#include <string.h>

#include "../../starkc/verbs.h"

#include "testresources.h"

#define STARKBANK_WIDGET_FIELDS(F)                                       \
/*    wire key         type              ref              flags       */ \
    F("amount",        AMOUNT,           NULL,  REQUIRED | CREATE | PATCH) \
    F("name",          STRING,           NULL,  REQUIRED | CREATE)      \
    F("rate",          RATE,             NULL,  CREATE)                 \
    F("expiration",    SECONDS,          NULL,  CREATE | PATCH)         \
    F("count",         NUMBER,           NULL,  CREATE)                 \
    F("active",        BOOL,             NULL,  CREATE | PATCH)         \
    F("day",           DATE,             NULL,  CREATE)                 \
    F("moment",        DATETIME,         NULL,  CREATE)                 \
    F("due",           DATE_OR_DATETIME, NULL,  CREATE | PATCH)         \
    F("tags",          LIST_STRING,      NULL,  CREATE)                 \
    F("discounts",     LIST_OBJECT,      NULL,  CREATE)                 \
    F("rules",         LIST_RESOURCE,    "Widget.Rule", CREATE)         \
    F("status",        STRING,           NULL,  PATCH)                  \
    F("metadata",      OBJECT,           NULL,  RO)                     \
    F("fee",           AMOUNT,           NULL,  RO)                     \
    F("id",            STRING,           NULL,  RO)                     \
    F("created",       DATETIME,         NULL,  RO)

#define STARKBANK_WIDGET_RULE_FIELDS(F)                                  \
    F("key",           STRING,           NULL,  REQUIRED | CREATE)       \
    F("value",         LIST_STRING,      NULL,  REQUIRED | CREATE)

#define STARKBANK_WIDGET_PAYMENT_FIELDS(F)                               \
    F("amount",        AMOUNT,           NULL,  RO)                      \
    F("name",          STRING,           NULL,  RO)

#define STARKBANK_WIDGET_LOG_FIELDS(F)                                   \
    F("id",            STRING,           NULL,  RO)                      \
    F("created",       DATETIME,         NULL,  RO)                      \
    F("type",          STRING,           NULL,  RO)                      \
    F("errors",        LIST_STRING,      NULL,  RO)                      \
    F("widget",        RESOURCE,         "Widget", RO)

/* The polymorphic case: log's table is named by the sibling subscription. */
#define STARKBANK_GADGET_FIELDS(F)                                       \
    F("id",            STRING,           NULL,  RO)                      \
    F("subscription",  STRING,           NULL,  RO)                      \
    F("isDelivered",   BOOL,             NULL,  PATCH)                   \
    F("log",           RESOURCE,         NULL,  RO)

#define STARKBANK_LEDGER_FIELDS(F)                                       \
    F("id",            STRING,           NULL,  RO)                      \
    F("amount",        AMOUNT,           NULL,  RO)

static const char *const widgetQuery[] = {
    "limit", "after", "before", "status", "tags", "ids", NULL
};
static const char *const widgetLogQuery[] = {
    "limit", "after", "before", "types", "widgetIds", NULL
};
static const char *const gadgetQuery[] = { "limit", "after", "before", NULL };

/*
 * Event.log in miniature: the nested table is chosen by the document, not by
 * the field row. An unknown subscription returns NULL, which leaves the nested
 * entity untagged and permissive rather than failing to hydrate.
 */
static const starkbankResource * gadgetLogResource(const starkcore_json *object,
                                                   const char *field)
{
    const starkcore_json *subscription;
    const char *value;

    if (strcmp(field, "log") != 0) {
        return NULL;
    }
    subscription = starkcore_json_get(object, "subscription");
    value = subscription != NULL ? starkcore_json_string(subscription) : NULL;
    if (value == NULL) {
        return NULL;
    }
    if (strcmp(value, "widget") == 0) {
        return starkbankRegistryFind("WidgetLog");
    }
    return NULL;
}

STARKBANK_RESOURCE(widget, "Widget", STARKBANK_WIDGET_FIELDS, widgetQuery);
STARKBANK_RESOURCE(widget_rule, "Widget.Rule", STARKBANK_WIDGET_RULE_FIELDS, NULL);
STARKBANK_RESOURCE(widget_payment, "Widget.Payment", STARKBANK_WIDGET_PAYMENT_FIELDS, NULL);
STARKBANK_RESOURCE(widget_log, "WidgetLog", STARKBANK_WIDGET_LOG_FIELDS, widgetLogQuery);
STARKBANK_RESOURCE_FULL(gadget, "Gadget", STARKBANK_GADGET_FIELDS, gadgetQuery, gadgetLogResource);
STARKBANK_RESOURCE(ledger, "Ledger", STARKBANK_LEDGER_FIELDS, NULL);

STARKBANK_VERB_NEW(widget)
STARKBANK_VERB_PARAMS(widget)
STARKBANK_VERB_POST_MULTI(widget)
STARKBANK_VERB_GET_ID(widget)
STARKBANK_VERB_QUERY(widget)
STARKBANK_VERB_PAGE(widget)
STARKBANK_VERB_PATCH_ID(widget)
STARKBANK_VERB_DELETE_ID(widget)
STARKBANK_VERB_CONTENT(widget, pdf)
STARKBANK_VERB_CONTENT_INT(widget, qrcode, "size", 1, 50)
STARKBANK_VERB_SUB_RESOURCE(widget, payment, "Payment", "Widget.Payment")

STARKBANK_VERB_NEW(widget_rule)

STARKBANK_VERB_PARAMS(widget_log)
STARKBANK_VERB_GET_ID(widget_log)
STARKBANK_VERB_QUERY(widget_log)

STARKBANK_VERB_GET_ID(gadget)

STARKBANK_VERB_GET_FIRST(ledger)
