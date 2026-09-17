/*
 * Judgement, not a REST call - same reason handwritten/event_parse.c exists,
 * and the one place the two differ is the envelope key.
 *
 * sdk-python's corporatepurchase.parse() calls parse_and_verify with key="",
 * and python's key argument is falsy-checked (`if key: json = json[key]`), so
 * an empty string means "the whole verified body IS the object", not "look up
 * the member named ''." starkcore_parse_and_verify spells that same "no
 * envelope" case with a NULL key, not an empty string - passing "" here would
 * make it search the body for a member literally named "", which no
 * CorporatePurchase authorization request has, and every call would fail with
 * STARKCORE_ERROR_MISSING_KEY. NULL is the correct translation of python's
 * falsy "", not a shortcut past it.
 */

#include <stddef.h>

#include <starkcore.h>

#include "internal.h"

extern const starkbankResource starkbankTable_corporate_purchase;

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_parse(const starkbank_client *client,
    const char *content, size_t content_len, const char *signature_base64,
    starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *document = NULL;
    int status;

    if (errors != NULL) {
        *errors = NULL;
    }
    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (starkbankClientCore(client) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_parse_and_verify(starkbankClientCore(client), content, content_len,
                                        signature_base64, NULL, &document, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankEntityWrap(&starkbankTable_corporate_purchase, document, document, out);
}
