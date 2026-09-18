/*
 * MerchantSession, its Purchase and AllowedInstallment sub-resources, and its
 * Log.
 *
 * purchase(uuid, purchase) is the reason this family needed new engine work.
 * sdk-python calls rest.post_sub_resource(resource=MerchantSession,
 * id=uuid, sub_resource=Purchase, entity=purchase): POST the given Purchase
 * to merchant-session/<uuid>/purchase, and unwrap the reply by PURCHASE's own
 * singular name, not MerchantSession's. Neither existing sub-path macro fits:
 * STARKBANK_VERB_POST_SINGLE_SUB (CorporateCard.create) posts to a literal
 * path segment with no id in the URL and unwraps by the OWNING resource's own
 * name; STARKBANK_VERB_SUB_RESOURCE (Invoice.payment) has an id in the URL
 * but is a GET with no body. core-c already has the exact shape python calls,
 * though - starkcore_rest_post_sub_resource, unused anywhere in this SDK
 * until now - so the new engine work is one shim
 * (starkbankVerbCreateSubResource in starkc/verb.c) plus one macro
 * (STARKBANK_VERB_POST_SUB_RESOURCE in starkc/verbs.h), both thin wrappers
 * over a call core-c already exposed; nothing in starkinfra/core-c changed.
 *
 * Purchase registers under its own bare python name ("Purchase"): its module
 * assigns a plain _resource = {"class": Purchase, "name": "Purchase"}, the
 * same shape Split and Permission use, and tools/drift.py's python reader
 * only qualifies a name when the OWNING module spells it _sub_resource.
 *
 * AllowedInstallment is the opposite case and registers as
 * "MerchantSession.AllowedInstallment", not bare "AllowedInstallment": its
 * module assigns _sub_resource = {"class": AllowedInstallment, "name":
 * "AllowedInstallment"}, and drift.py's readPython() qualifies every
 * _sub_resource by the resource name of the package that owns it - the exact
 * mechanism that makes "Rule" become "Invoice.Rule"/"Transfer.Rule"/
 * "BrcodePayment.Rule" wherever three different resources each embed their
 * own same-named Rule. Whether the bare name happens to collide anywhere
 * else is irrelevant to the check: it keys off _resource vs _sub_resource,
 * not off name collision, so this table follows suit even though nothing
 * else in the SDK is named "AllowedInstallment".
 *
 * Both files live in this directory because MerchantSession is the only
 * family that embeds them, not because the registry says so - the same
 * placement corporateholder/permission.c documents for Permission.
 *
 * Purchase is a Resource (it has an id, returned once the API creates it, and
 * python's Resource.__init__ stores it) even though nothing ever calls
 * starkbank_purchase_get/query/page on it - sdk-python's Purchase module
 * exports no such functions, so this table gets no GET_ID/QUERY/PAGE verb to
 * match, only STARKBANK_VERB_NEW to build one to hand to
 * starkbank_merchant_session_purchase.
 *
 * Purchase.metadata is the first CREATE-writable single OBJECT field in this
 * SDK - every earlier OBJECT field (Transfer.metadata) is RO. A single
 * OBJECT has no dedicated typed setter (only LIST_OBJECT/LIST_RESOURCE
 * fields take starkbank_entity_append_entity); a caller writes it with the
 * documented escape hatch, starkbank_entity_set_json_raw(purchase,
 * STARKBANK_PURCHASE_METADATA, "{...}"), still validated at dehydrate time
 * because the table already declares the key CREATE - no new engine shape
 * needed. MerchantPurchase.metadata is the same shape.
 *
 * MerchantSessionLog.errors is LIST_STRING: the merchant-session/log endpoint
 * is served by the acquirer service, whose Log.errors is a list of plain
 * strings on the wire, unlike MerchantCard/MerchantInstallment/
 * MerchantPurchase's logs (LIST_OBJECT {code,message} pairs, the same shape
 * CorporatePurchase.Log already established) - the two log families come
 * from different backing services and are not the same wire shape.
 *
 * expiration is NUMBER, not SECONDS, even though the docstring calls it
 * "integer or datetime.timedelta" the same way Invoice.expiration is worded:
 * unlike Invoice.expiration, MerchantSession's __init__ does
 * "self.expiration = expiration" with no check_timedelta call at all, so
 * python sends back exactly whatever integer it was given and never accepts
 * a timedelta in practice. tools/drift.py's type.conflict check is normative
 * on python's actual coercion, not its docstring's prose, and is a hard stop
 * that no known-drift entry can silence - so the table follows the real
 * behaviour rather than the naive per-field mirror this SDK would otherwise
 * always reach for.
 *
 * holderId rides in MerchantSession's query key list even though the docs'
 * GET /v2/merchant-session parameter list does not mention it: sdk-python's
 * query()/page() both take holder_id and forward it, and python is normative
 * for the verb surface (see tests/reference/known-drift.json's
 * query.gone:MerchantSession:holderId).
 */

#ifndef STARKBANK_MERCHANTSESSION_H
#define STARKBANK_MERCHANTSESSION_H

#define STARKBANK_MERCHANT_SESSION_FIELDS(F)                                          \
/*    wire key                type               ref                    flags     */ \
    F("allowedFundingTypes",  LIST_STRING,       NULL,  REQUIRED | CREATE)             \
    F("allowedInstallments",  LIST_RESOURCE,     "MerchantSession.AllowedInstallment",  \
                                                         REQUIRED | CREATE)             \
    F("expiration",           NUMBER,            NULL,  REQUIRED | CREATE)             \
    F("allowedIps",           LIST_STRING,       NULL,  CREATE)                        \
    F("challengeMode",        STRING,            NULL,  CREATE)                        \
    F("tags",                 LIST_STRING,       NULL,  CREATE)                        \
    F("id",                   STRING,            NULL,  RO)                            \
    F("uuid",                 STRING,            NULL,  RO)                            \
    F("holderId",             STRING,            NULL,  RO)                            \
    F("softDescriptor",       STRING,            NULL,  RO)                            \
    F("status",               STRING,            NULL,  RO)                            \
    F("created",              DATETIME,          NULL,  RO)                            \
    F("updated",              DATETIME,          NULL,  RO)

/* SubResource in python (no id); embedded in MerchantSession.allowedInstallments. */
#define STARKBANK_ALLOWED_INSTALLMENT_FIELDS(F)                                       \
    F("totalAmount",          AMOUNT,            NULL,  REQUIRED | CREATE)             \
    F("count",                NUMBER,            NULL,  REQUIRED | CREATE)

/* Resource in python (has an id); built with starkbank_purchase_new and sent
   through starkbank_merchant_session_purchase - see this file's header. */
#define STARKBANK_PURCHASE_FIELDS(F)                                                  \
/*    wire key                type               ref     flags                    */ \
    F("amount",                AMOUNT,    NULL,  REQUIRED | CREATE)                    \
    F("cardExpiration",        STRING,    NULL,  REQUIRED | CREATE)                    \
    F("cardNumber",            STRING,    NULL,  REQUIRED | CREATE)                    \
    F("cardSecurityCode",      STRING,    NULL,  REQUIRED | CREATE)                    \
    F("holderName",            STRING,    NULL,  REQUIRED | CREATE)                    \
    F("fundingType",           STRING,    NULL,  REQUIRED | CREATE)                    \
    F("holderEmail",           STRING,    NULL,  CREATE)                               \
    F("holderPhone",           STRING,    NULL,  CREATE)                               \
    F("holderId",              STRING,    NULL,  CREATE)                               \
    F("installmentCount",      NUMBER,    NULL,  CREATE)                               \
    F("billingCountryCode",    STRING,    NULL,  CREATE)                               \
    F("billingCity",           STRING,    NULL,  CREATE)                               \
    F("billingStateCode",      STRING,    NULL,  CREATE)                               \
    F("billingStreetLine1",    STRING,    NULL,  CREATE)                               \
    F("billingStreetLine2",    STRING,    NULL,  CREATE)                               \
    F("billingZipCode",        STRING,    NULL,  CREATE)                               \
    F("metadata",              OBJECT,    NULL,  CREATE)                               \
    F("softDescriptor",        STRING,    NULL,  CREATE)                               \
    F("tags",                  LIST_STRING, NULL, CREATE)                              \
    F("id",                    STRING,    NULL,  RO)                                   \
    F("cardEnding",            STRING,    NULL,  RO)                                   \
    F("cardId",                STRING,    NULL,  RO)                                   \
    F("challengeMode",         STRING,    NULL,  RO)                                   \
    F("challengeUrl",          STRING,    NULL,  RO)                                   \
    F("currencyCode",          STRING,    NULL,  RO)                                   \
    F("endToEndId",            STRING,    NULL,  RO)                                   \
    F("fee",                   AMOUNT,    NULL,  RO)                                   \
    F("network",               STRING,    NULL,  RO)                                   \
    F("source",                STRING,    NULL,  RO)                                   \
    F("status",                STRING,    NULL,  RO)                                   \
    F("created",               DATETIME,  NULL,  RO)                                   \
    F("updated",                DATETIME, NULL,  RO)

#define STARKBANK_MERCHANT_SESSION_LOG_FIELDS(F)                                      \
    F("id",                   STRING,            NULL,  RO)                            \
    F("created",              DATETIME,          NULL,  RO)                            \
    F("type",                 STRING,            NULL,  RO)                            \
    F("errors",               LIST_STRING,       NULL,  RO)                            \
    F("session",              RESOURCE,          "MerchantSession", RO)

#endif /* STARKBANK_MERCHANTSESSION_H */
