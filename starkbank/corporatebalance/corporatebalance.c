#include "../../starkc/verbs.h"

#include "corporatebalance.h"

STARKBANK_RESOURCE(corporate_balance, "CorporateBalance", STARKBANK_CORPORATE_BALANCE_FIELDS,
                   NULL);

/* python reads the listing endpoint and takes its head; so does this. */
STARKBANK_VERB_GET_FIRST(corporate_balance)
