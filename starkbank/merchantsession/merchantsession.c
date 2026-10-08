#include "../../starkc/verbs.h"

#include "merchantsession.h"

static const char *const merchantSessionQuery[] = {
    "limit", "status", "tags", "ids", "after", "before", "holderId", NULL
};

STARKBANK_RESOURCE(merchant_session, "MerchantSession", STARKBANK_MERCHANT_SESSION_FIELDS,
                   merchantSessionQuery);

STARKBANK_VERB_NEW(merchant_session)
STARKBANK_VERB_PARAMS(merchant_session)
STARKBANK_VERB_POST_SINGLE(merchant_session)
STARKBANK_VERB_GET_ID(merchant_session)
STARKBANK_VERB_QUERY(merchant_session)
STARKBANK_VERB_PAGE(merchant_session)
