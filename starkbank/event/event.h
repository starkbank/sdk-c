/*
 * Event: the hardest hydration case in either SDK, and deliberately in the
 * first slice rather than the ninetieth.
 *
 * log has no ref in its row. Which table it hydrates as is written in the
 * sibling "subscription" field, so the resource carries a variant map and the
 * engine asks the document. Nothing about that is Event-specific: any resource
 * whose nested shape is chosen by a discriminator carries the same map, which
 * is why PaymentPreview.payment needed no engine change and no code at all.
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

/*
 * sdk-python's _resource_by_subscription, verbatim and in its order. Eight of
 * these ten tables arrive with the rest of the bank surface; until then
 * starkbankRegistryFind returns NULL for them and the log hydrates untagged
 * and permissive, with the unknown counter reporting the gap. That is the same
 * path a subscription invented after this build ships will take, so the case
 * is exercised on every run rather than waiting for the API to grow one.
 */
#define STARKBANK_EVENT_LOG_VARIANTS(V)                                              \
    V("transfer",        "TransferLog")                                              \
    V("invoice",         "InvoiceLog")                                               \
    V("deposit",         "DepositLog")                                               \
    V("boleto",          "BoletoLog")                                                \
    V("brcode-payment",  "BrcodePaymentLog")                                         \
    V("boleto-payment",  "BoletoPaymentLog")                                         \
    V("utility-payment", "UtilityPaymentLog")                                        \
    V("darf-payment",    "DarfPaymentLog")                                           \
    V("tax-payment",     "TaxPaymentLog")                                            \
    V("holmes",          "BoletoHolmesLog")

/* Resource "EventAttempt"; the -attempt rewrite to event/attempt is core-c's. */
#define STARKBANK_EVENT_ATTEMPT_FIELDS(F)                                            \
    F("id",              STRING,            NULL,  RO)                               \
    F("code",            STRING,            NULL,  RO)                               \
    F("message",         STRING,            NULL,  RO)                               \
    F("eventId",         STRING,            NULL,  RO)                               \
    F("webhookId",       STRING,            NULL,  RO)                               \
    F("created",         DATETIME,          NULL,  RO)

#endif /* STARKBANK_EVENT_H */
