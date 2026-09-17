/*
 * sdk-python's corporatepurchase.response() is not a REST call at all - it
 * builds {"authorization": {...}} and returns json.dumps(api_json(...)), a
 * plain string a caller's own webhook handler writes back as its HTTP
 * response body. There is no network call, no signing, and nothing here
 * needs anything starkcore does not already export publicly
 * (starkcore_json_*, starkcore_api_cast, starkcore_json_print), so this is
 * hand-written rather than a verb macro for the same reason event_parse.c is:
 * no STARKBANK_VERB_* shape is "build and print a JSON object with no wire
 * call," and manufacturing one for this single caller would be a macro with
 * exactly one user.
 *
 * tags is a caller-joined comma-separated string rather than a pointer array,
 * for the reason STARKBANK_VERB_CONTENT_QUERY's hiddenFields is: one string
 * parameter is both the simplest ABI shape and the one every binding
 * generator already knows how to translate. amount is a presence flag plus a
 * plain double rather than a nullable "const double *", for the same
 * reason: tools/emit.py's PASCAL_TYPES/CSHARP_TYPES tables translate every
 * type this header uses, and a bare optional-scalar pointer is a shape no
 * other verb in this library has ever needed - has_amount says what "" says
 * for a string, without teaching two binding generators a new pointer type
 * for one caller.
 *
 * A field python's api_json would drop for being None is left out of the
 * object entirely here, never sent as a JSON null: status and reason absent,
 * amount NULL, tags NULL or empty all mean "do not send this key."
 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <starkcore.h>

#include "internal.h"

/* The same "no starkcore_json_new_string" workaround
   starkbank_entity_append_string uses: a one-member object is the shortest
   honest way to get an owned scalar node out of the ABI. */
static int newStringNode(const char *value, starkcore_json **out)
{
    starkcore_json *holder = NULL;
    const starkcore_json *source;
    int status = starkcore_json_new_object(&holder);

    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_json_set_string(holder, "v", value);
    if (status != STARKCORE_OK) {
        starkcore_json_free(holder);
        return status;
    }
    source = starkcore_json_get(holder, "v");
    status = starkcore_json_clone(source, out);
    starkcore_json_free(holder);
    return status;
}

STARKBANK_API int STARKBANK_CALL starkbank_corporate_purchase_response(const char *status,
    int has_amount, double amount, const char *reason, const char *tags, char **out)
{
    starkcore_json *authorization = NULL;
    starkcore_json *envelope = NULL;
    starkcore_json *cast = NULL;
    starkcore_json *array = NULL;
    starkcore_json *element = NULL;
    int result;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    result = starkcore_json_new_object(&authorization);
    if (result == STARKCORE_OK && status != NULL && status[0] != '\0') {
        result = starkcore_json_set_string(authorization, "status", status);
    }
    if (result == STARKCORE_OK && has_amount) {
        result = starkcore_json_set_number(authorization, "amount", amount);
    }
    if (result == STARKCORE_OK && reason != NULL && reason[0] != '\0') {
        result = starkcore_json_set_string(authorization, "reason", reason);
    }
    if (result == STARKCORE_OK && tags != NULL && tags[0] != '\0') {
        result = starkcore_json_new_array(&array);
        if (result == STARKCORE_OK) {
            const char *cursor = tags;
            const char *comma;
            size_t length;

            while (result == STARKCORE_OK && *cursor != '\0') {
                comma = strchr(cursor, ',');
                length = comma != NULL ? (size_t)(comma - cursor) : strlen(cursor);
                if (length > 0) {
                    char *segment = (char *)malloc(length + 1);
                    if (segment == NULL) {
                        result = STARKCORE_ERROR_MEMORY;
                    } else {
                        memcpy(segment, cursor, length);
                        segment[length] = '\0';
                        result = newStringNode(segment, &element);
                        free(segment);
                        if (result == STARKCORE_OK) {
                            result = starkcore_json_append(array, element);
                        }
                    }
                }
                cursor = comma != NULL ? comma + 1 : cursor + length;
            }
        }
        if (result == STARKCORE_OK) {
            result = starkcore_json_set_json(authorization, "tags", array);
            array = NULL;
        }
    }
    if (result == STARKCORE_OK) {
        result = starkcore_json_new_object(&envelope);
    }
    if (result == STARKCORE_OK) {
        result = starkcore_json_set_json(envelope, "authorization", authorization);
        authorization = NULL;
    }
    if (result == STARKCORE_OK) {
        result = starkcore_api_cast(envelope, &cast);
    }
    starkcore_json_free(array);
    starkcore_json_free(authorization);
    starkcore_json_free(envelope);
    if (result != STARKCORE_OK) {
        starkcore_json_free(cast);
        return result;
    }
    {
        size_t length = 0;
        result = starkcore_json_print(cast, out, &length);
    }
    starkcore_json_free(cast);
    return result;
}
