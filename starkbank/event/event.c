#include "../../starkc/verbs.h"

#include "event.h"

static const char *const eventQuery[] = { "limit", "after", "before", "isDelivered", NULL };

STARKBANK_POLYMORPH(event, "log", "subscription", STARKBANK_EVENT_LOG_VARIANTS);

STARKBANK_RESOURCE_FULL(event, "Event", STARKBANK_EVENT_FIELDS, eventQuery,
                        starkbankPolymorph_event);

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
