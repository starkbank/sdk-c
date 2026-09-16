#include "../../starkc/verbs.h"

#include "event.h"

static const char *const eventAttemptQuery[] = {
    "limit", "after", "before", "eventIds", "webhookIds", NULL
};

STARKBANK_RESOURCE(event_attempt, "EventAttempt", STARKBANK_EVENT_ATTEMPT_FIELDS,
                   eventAttemptQuery);

STARKBANK_VERB_PARAMS(event_attempt)
STARKBANK_VERB_GET_ID(event_attempt)
STARKBANK_VERB_QUERY(event_attempt)
STARKBANK_VERB_PAGE(event_attempt)
