#include "../../starkc/verbs.h"

#include "paymentpreview.h"

STARKBANK_POLYMORPH(payment_preview, "payment", "type", STARKBANK_PAYMENT_PREVIEW_VARIANTS);

STARKBANK_RESOURCE_FULL(payment_preview, "PaymentPreview", STARKBANK_PAYMENT_PREVIEW_FIELDS,
                        NULL, starkbankPolymorph_payment_preview);

STARKBANK_VERB_NEW(payment_preview)
STARKBANK_VERB_POST_MULTI(payment_preview)
/* "PaymentPreview" becomes /v2/payment-preview and the payload key "previews"
   at run time, both in starkcore_api_*. Neither is spelled here. */
