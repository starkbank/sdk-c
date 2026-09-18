/* A Resource (has an id) with no GET/QUERY/PAGE of its own: sdk-python's
   Purchase module exports none. Built with starkbank_purchase_new and sent
   through starkbank_merchant_session_purchase - see merchantsession.h. */

#include "../../starkc/verbs.h"

#include "merchantsession.h"

STARKBANK_RESOURCE(purchase, "Purchase", STARKBANK_PURCHASE_FIELDS, NULL);

STARKBANK_VERB_NEW(purchase)
