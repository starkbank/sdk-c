#include "../../starkc/verbs.h"

#include "balance.h"

STARKBANK_RESOURCE(balance, "Balance", STARKBANK_BALANCE_FIELDS, NULL);

/* python reads the listing endpoint and takes its head; so does this. */
STARKBANK_VERB_GET_FIRST(balance)
