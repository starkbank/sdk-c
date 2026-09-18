/*
 * VerifiedAccount and its Log.
 *
 * sdk-python spells the DELETE /v2/verified-account/:id call cancel();
 * sdk-c keeps every resource's DELETE_ID verb named starkbank_<resource>_delete
 * for one consistent spelling across the whole SDK, the same choice already
 * made for CorporateHolder.cancel()/CorporateCard.cancel() - see
 * tests/reference/known-drift.json's verb.new/verb.gone:VerifiedAccount pair.
 * The HTTP call is identical; only the C symbol's name differs from python's.
 *
 * taxId rides in the docs' GET /v2/verified-account parameter list but not in
 * this table's query keys: sdk-python's query()/page() both take limit,
 * after, before, status, ids and tags and forward exactly those to
 * rest.get_stream/get_page, with no tax_id keyword anywhere in either
 * signature. python is normative for the verb surface, so the table does not
 * add a filter no SDK call actually sends - see
 * tests/reference/known-drift.json's query.new:VerifiedAccount:taxId.
 *
 * VerifiedAccountLog.errors is LIST_OBJECT, not LIST_STRING despite
 * sdk-python's own docstring calling it "list of strings": app-docs'
 * verified-account.js sample shows the wire shape directly -
 * "errors": [{"code": "keyNotFound", "message": "The key is not
 * registered"}] - and the api-v2-ms-transfer service that backs
 * VerifiedAccount confirms it in code, not just in a sample: models/
 * verifiedAccountLog.py's errorDescriptions (lines 37-42) builds exactly
 * [{"code": error, "message": ...} for error in self.errors], and json()
 * serialises the field through it at line 49 - the same {code, message}
 * shape CorporatePurchase.Log already established for LIST_OBJECT errors.
 * sdk-python's own __init__ does a plain "self.errors = errors" with no
 * coercion either way, so nothing in its code contradicts this - only its
 * docstring's prose does, and prose loses.
 */

#ifndef STARKBANK_VERIFIEDACCOUNT_H
#define STARKBANK_VERIFIEDACCOUNT_H

#define STARKBANK_VERIFIED_ACCOUNT_FIELDS(F)                                         \
/*    wire key       type               ref                 flags               */  \
    F("taxId",       STRING,            NULL,  REQUIRED | CREATE)                    \
    F("bankCode",    STRING,            NULL,  CREATE)                               \
    F("branchCode",  STRING,            NULL,  CREATE)                               \
    F("keyId",       STRING,            NULL,  CREATE)                               \
    F("name",        STRING,            NULL,  CREATE)                               \
    F("number",      STRING,            NULL,  CREATE)                               \
    F("type",        STRING,            NULL,  CREATE)                               \
    F("tags",        LIST_STRING,       NULL,  CREATE)                               \
    F("id",          STRING,            NULL,  RO)                                   \
    F("bankName",    STRING,            NULL,  RO)                                   \
    F("status",      STRING,            NULL,  RO)                                   \
    F("created",     DATETIME,          NULL,  RO)                                   \
    F("updated",     DATETIME,          NULL,  RO)

#define STARKBANK_VERIFIED_ACCOUNT_LOG_FIELDS(F)                                     \
    F("id",          STRING,            NULL,  RO)                                   \
    F("created",     DATETIME,          NULL,  RO)                                   \
    F("type",        STRING,            NULL,  RO)                                   \
    F("errors",      LIST_OBJECT,       NULL,  RO)                                   \
    F("account",     RESOURCE,          "VerifiedAccount", RO)

#endif /* STARKBANK_VERIFIEDACCOUNT_H */
