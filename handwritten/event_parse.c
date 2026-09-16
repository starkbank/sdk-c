/*
 * The one verb in the first slice with judgement in it, and the only file in
 * the library a verb macro will never write.
 *
 * It is here because it is not a REST call at all: nothing about verifying a
 * webhook body fits a starkcore_rest_* shape, so there is no macro it could
 * be an instance of. The design's rule is that such a method is declared in
 * the public header and defined only by hand - delete this file and the link
 * fails with an undefined symbol, which is an instruction nobody can ignore.
 *
 * The table is referenced directly rather than looked up by name. The generic
 * starkbank_parse_and_verify in starkc/facade.c asks the registry, because the
 * engine cannot know Event exists; here it can, and a compile-time reference
 * means a body can never quietly hydrate as an untagged map because somebody
 * dropped a registry line.
 */

#include <stddef.h>

#include <starkcore.h>

#include "internal.h"

extern const starkbankResource starkbankTable_event;

STARKBANK_API int STARKBANK_CALL starkbank_event_parse(const starkbank_client *client,
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
    /* The "event" envelope key is supplied here so a host never has to know
       the webhook body is wrapped. Verification happens first: on a bad
       signature nothing is parsed and nothing is handed back. */
    status = starkcore_parse_and_verify(starkbankClientCore(client), content, content_len,
                                        signature_base64, "event", &document, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankEntityWrap(&starkbankTable_event, document, document, out);
}
