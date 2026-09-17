/*
 * Webhook: the only bank resource created one at a time.
 *
 * sdk-python's webhook.create calls rest.post_single, whose body is the
 * entity itself - no plural list, and no singular envelope key either; the
 * singular key is what the RESPONSE is unwrapped from, in core-c. The verb
 * macro is STARKBANK_VERB_POST_SINGLE for that reason and not because a batch
 * of one would be wrong on the wire.
 *
 * url and subscriptions are both required: python's __init__ takes them
 * positionally and its docstring files them under Parameters (required), and
 * drift.py's flag.conflict check holds the table to that.
 */

#ifndef STARKBANK_WEBHOOK_H
#define STARKBANK_WEBHOOK_H

#define STARKBANK_WEBHOOK_FIELDS(F)                                                  \
/*    wire key           type               ref                 flags            */ \
    F("url",             STRING,            NULL,  REQUIRED | CREATE)                \
    F("subscriptions",   LIST_STRING,       NULL,  REQUIRED | CREATE)                \
    F("id",              STRING,            NULL,  RO)

#endif /* STARKBANK_WEBHOOK_H */
