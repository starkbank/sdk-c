#include "../../starkc/verbs.h"

#include "webhook.h"

static const char *const webhookQuery[] = { "limit", NULL };

STARKBANK_RESOURCE(webhook, "Webhook", STARKBANK_WEBHOOK_FIELDS, webhookQuery);

STARKBANK_VERB_NEW(webhook)
STARKBANK_VERB_PARAMS(webhook)
STARKBANK_VERB_POST_SINGLE(webhook)
STARKBANK_VERB_GET_ID(webhook)
STARKBANK_VERB_QUERY(webhook)
STARKBANK_VERB_PAGE(webhook)
STARKBANK_VERB_DELETE_ID(webhook)
