#include "../../starkc/verbs.h"

#include "cardmethod.h"

/* No "limit": sdk-python's query() does not take one. See cardmethod.h. */
static const char *const cardMethodQuery[] = { "search", NULL };

STARKBANK_RESOURCE(card_method, "CardMethod", STARKBANK_CARD_METHOD_FIELDS, cardMethodQuery);

STARKBANK_VERB_PARAMS(card_method)
STARKBANK_VERB_QUERY(card_method)
