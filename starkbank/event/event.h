/*
 * Event: the hardest hydration case in either SDK, and deliberately in the
 * first slice rather than the ninetieth.
 *
 * log has no ref in its row. Which table it hydrates as is written in the
 * sibling "subscription" field, so the resource carries a starkbankRefFn and
 * the engine asks the document. Nothing about that is Event-specific: any
 * resource whose nested shape is chosen by a discriminator uses the same hook.
 */

#ifndef STARKBANK_EVENT_H
#define STARKBANK_EVENT_H

#define STARKBANK_EVENT_FIELDS(F)                                                    \
    F("id",              STRING,            NULL,  RO)                               \
    F("log",             RESOURCE,          NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)                               \
    F("isDelivered",     BOOL,              NULL,  PATCH)                            \
    F("subscription",    STRING,            NULL,  RO)                               \
    F("workspaceId",     STRING,            NULL,  RO)

/* Resource "EventAttempt"; the -attempt rewrite to event/attempt is core-c's. */
#define STARKBANK_EVENT_ATTEMPT_FIELDS(F)                                            \
    F("id",              STRING,            NULL,  RO)                               \
    F("code",            STRING,            NULL,  RO)                               \
    F("message",         STRING,            NULL,  RO)                               \
    F("eventId",         STRING,            NULL,  RO)                               \
    F("webhookId",       STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)

#endif /* STARKBANK_EVENT_H */
