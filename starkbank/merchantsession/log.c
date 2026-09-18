/* No pdf verb: sdk-python's merchantsession.Log has none. errors is
   LIST_STRING, not LIST_OBJECT - see merchantsession.h. */

#include "../../starkc/verbs.h"

#include "merchantsession.h"

static const char *const merchantSessionLogQuery[] = {
    "limit", "after", "before", "types", "sessionIds", NULL
};

STARKBANK_RESOURCE(merchant_session_log, "MerchantSessionLog",
                   STARKBANK_MERCHANT_SESSION_LOG_FIELDS, merchantSessionLogQuery);

STARKBANK_VERB_PARAMS(merchant_session_log)
STARKBANK_VERB_GET_ID(merchant_session_log)
STARKBANK_VERB_QUERY(merchant_session_log)
STARKBANK_VERB_PAGE(merchant_session_log)
