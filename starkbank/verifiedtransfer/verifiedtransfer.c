/* create only, over post_multi - see verifiedtransfer.h for why there is no
   query key list and no PARAMS/GET_ID/QUERY/PAGE verb here. */

#include "../../starkc/verbs.h"

#include "verifiedtransfer.h"

STARKBANK_RESOURCE(verified_transfer, "VerifiedTransfer", STARKBANK_VERIFIED_TRANSFER_FIELDS,
                   NULL);

STARKBANK_VERB_NEW(verified_transfer)
STARKBANK_VERB_POST_MULTI(verified_transfer)
