/*
 * What the verb macros expand into a call to. Each function is the same three
 * steps in the same order - validate the arguments, hand the work to a
 * starkcore_rest_* or starkcore_stream_* call, wrap what comes back - and
 * nothing here knows an endpoint, an envelope key or a pagination rule. Those
 * are core-c's, resolved at run time from the resource name, which is why
 * "InvoiceLog" becoming "invoice/log" is not spelled anywhere in this repo.
 */

#include <stdlib.h>
#include <string.h>

#include "internal.h"

static int wrapOwned(const starkbankResource *resource, starkcore_json *document,
                     starkbank_entity **out)
{
    return starkbankEntityWrap(resource, document, document, out);
}

static int checkVerb(const starkbank_client *client, starkbank_errors **errors)
{
    if (errors != NULL) {
        *errors = NULL;
    }
    if (starkbankClientCore(client) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    return STARKCORE_OK;
}

int starkbankVerbCreate(const starkbank_client *client, const starkbankResource *resource,
                        const starkbank_list *entities, starkbank_list **out,
                        starkbank_errors **errors)
{
    const starkbank_entity *entity;
    starkcore_json *payload = NULL;
    starkcore_json *item = NULL;
    starkcore_json *reply = NULL;
    int status;
    int index;
    int count;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    count = starkbank_list_count(entities);
    if (count < 0) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_json_new_array(&payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    for (index = 0; index < count; index++) {
        entity = starkbank_list_at(entities, index);
        /* Compared by table pointer, not by name through the registry: a
           resource whose registry line someone forgot must still create. */
        if (entity == NULL || entity->resource != resource) {
            starkcore_json_free(payload);
            return STARKBANK_ERROR_RESOURCE;
        }
        /* Required keys are checked before anything is sent: a missing one
           costs a local error rather than a round trip and a 400. */
        status = starkbankEntityDehydrate(entity, STARKBANK_FLAG_CREATE, &item);
        if (status != STARKCORE_OK) {
            starkcore_json_free(payload);
            return status;
        }
        status = starkcore_json_append(payload, item);
        if (status != STARKCORE_OK) {
            starkcore_json_free(payload);
            return status;
        }
    }
    status = starkcore_rest_post_multi(starkbankClientCore(client), resource->name,
                                       payload, NULL, &reply, errors);
    starkcore_json_free(payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankListFromJson(resource, reply, out);
}

/*
 * rest.post_single. One entity, and the body is that entity - python's
 * post_single sends api_json(entity) itself, with no envelope key and no
 * one-element list, and core-c's starkcore_rest_post_single does the same.
 * The singular envelope key is on the RESPONSE, where core-c unwraps it.
 * tests/reference/slice.json records python's own bytes for webhook.create,
 * which is the only reason this comment can be believed.
 */
int starkbankVerbCreateSingle(const starkbank_client *client, const starkbankResource *resource,
                              const starkbank_entity *entity, starkbank_entity **out,
                              starkbank_errors **errors)
{
    starkcore_json *payload = NULL;
    starkcore_json *reply = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    /* Through the public reader, which checks the handle: this entity came
       straight from a caller rather than out of a list we built. */
    if (starkbank_entity_json(entity) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (entity->resource != resource) {
        return STARKBANK_ERROR_RESOURCE;
    }
    status = starkbankEntityDehydrate(entity, STARKBANK_FLAG_CREATE, &payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_rest_post_single(starkbankClientCore(client), resource->name,
                                        payload, NULL, &reply, errors);
    starkcore_json_free(payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, reply, out);
}

/*
 * CorporateCard.create's shape: rest.post_raw to endpoint(resource) + "/" +
 * subPath, with no id in the path and the envelope key the RESOURCE's own
 * singular name rather than a sibling's - sendSingle in core-c's rest.c comes
 * within one field of this (it already skips the id segment when id is NULL)
 * but always unwraps by the "sub" resource's name when a sub is given, which
 * is right for starkcore_rest_post_sub_resource's real callers and wrong
 * here. No existing starkcore_rest_* call fits both halves at once, and nothing
 * about this needs core-c to change: it composes public entry points
 * (starkcore_api_endpoint, starkcore_api_last_name, starkcore_api_cast,
 * starkcore_fetch, starkcore_response_*) the exact way sendSingle does
 * internally, just with the literal path segment sendSingle cannot take.
 */
int starkbankVerbCreateSub(const starkbank_client *client, const starkbankResource *resource,
                           const char *subPath, const starkbank_entity *entity,
                           starkbank_entity **out, starkbank_errors **errors)
{
    char *endpoint = NULL;
    char *lastName = NULL;
    char *path = NULL;
    starkcore_json *dehydrated = NULL;
    starkcore_json *cast = NULL;
    starkcore_response *response = NULL;
    starkcore_json *envelope = NULL;
    starkcore_json *copy = NULL;
    const starkcore_json *member;
    size_t length;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (starkbank_entity_json(entity) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (entity->resource != resource) {
        return STARKBANK_ERROR_RESOURCE;
    }
    status = starkbankEntityDehydrate(entity, STARKBANK_FLAG_CREATE, &dehydrated);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_api_cast(dehydrated, &cast);
    starkcore_json_free(dehydrated);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_api_endpoint(resource->name, &endpoint);
    if (status == STARKCORE_OK) {
        status = starkcore_api_last_name(resource->name, &lastName);
    }
    if (status == STARKCORE_OK) {
        length = strlen(endpoint) + 1 + strlen(subPath) + 1;
        path = (char *)malloc(length);
        if (path == NULL) {
            status = STARKCORE_ERROR_MEMORY;
        } else {
            strcpy(path, endpoint);
            strcat(path, "/");
            strcat(path, subPath);
        }
    }
    if (status == STARKCORE_OK) {
        status = starkcore_fetch(starkbankClientCore(client), STARKCORE_METHOD_POST, path,
                                 cast, NULL, NULL, &response);
    }
    starkcore_json_free(cast);
    free(endpoint);
    free(path);
    if (status != STARKCORE_OK) {
        free(lastName);
        return status;
    }
    status = starkcore_response_check(response, errors);
    if (status == STARKCORE_OK) {
        status = starkcore_response_json(response, &envelope);
    }
    starkcore_response_free(response);
    if (status != STARKCORE_OK) {
        free(lastName);
        return status;
    }
    member = starkcore_json_get(envelope, lastName);
    free(lastName);
    if (member == NULL) {
        starkcore_json_free(envelope);
        return STARKCORE_ERROR_MISSING_KEY;
    }
    status = starkcore_json_clone(member, &copy);
    starkcore_json_free(envelope);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, copy, out);
}

int starkbankVerbGetId(const starkbank_client *client, const starkbankResource *resource,
                       const char *id, starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *reply = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_rest_get_id(starkbankClientCore(client), resource->name, id,
                                   NULL, &reply, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, reply, out);
}

/*
 * Balance has no id and python reads the listing endpoint and takes its head.
 * The head is cloned out of the page so the caller owns one object rather than
 * a page it must remember is underneath.
 */
int starkbankVerbGetFirst(const starkbank_client *client, const starkbankResource *resource,
                          starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *reply = NULL;
    starkcore_json *copy = NULL;
    char *cursor = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_rest_get_page(starkbankClientCore(client), resource->name, NULL,
                                     &reply, &cursor, errors);
    starkcore_free(cursor);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (starkcore_json_size(reply) < 1) {
        starkcore_json_free(reply);
        return STARKBANK_ERROR_ABSENT;
    }
    status = starkcore_json_clone(starkcore_json_at(reply, 0), &copy);
    starkcore_json_free(reply);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, copy, out);
}

int starkbankVerbQuery(const starkbank_client *client, const starkbankResource *resource,
                       const starkbank_entity *params, int limit, starkbank_iter **out)
{
    starkcore_stream *stream = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (starkbankClientCore(client) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_stream_new(starkbankClientCore(client), resource->name,
                                  starkbank_entity_json(params), limit, &stream);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankIterNew(stream, resource, out);
}

int starkbankVerbPage(const starkbank_client *client, const starkbankResource *resource,
                      const starkbank_entity *params, starkbank_list **out,
                      char **outCursor, starkbank_errors **errors)
{
    starkcore_json *reply = NULL;
    char *cursor = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (outCursor != NULL) {
        *outCursor = NULL;
    }
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_rest_get_page(starkbankClientCore(client), resource->name,
                                     starkbank_entity_json(params), &reply, &cursor, errors);
    if (status != STARKCORE_OK) {
        starkcore_free(cursor);
        return status;
    }
    status = starkbankListFromJson(resource, reply, out);
    if (status != STARKCORE_OK) {
        starkcore_free(cursor);
        return status;
    }
    if (outCursor != NULL) {
        *outCursor = cursor;
        return STARKCORE_OK;
    }
    starkcore_free(cursor);
    return STARKCORE_OK;
}

int starkbankVerbPatchId(const starkbank_client *client, const starkbankResource *resource,
                         const char *id, const starkbank_entity *patch,
                         starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *payload = NULL;
    starkcore_json *reply = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL || patch == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkbankEntityDehydrate(patch, STARKBANK_FLAG_PATCH, &payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_rest_patch_id(starkbankClientCore(client), resource->name, id,
                                     payload, NULL, &reply, errors);
    starkcore_json_free(payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, reply, out);
}

int starkbankVerbPatchIdEcho(const starkbank_client *client, const starkbankResource *resource,
                             const char *id, const starkbank_entity *patch,
                             const char *const *echoKeys,
                             starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *payload = NULL;
    starkcore_json *query = NULL;
    starkcore_json *reply = NULL;
    const char *value;
    int index;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL || patch == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkbankEntityDehydrate(patch, STARKBANK_FLAG_PATCH, &payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    for (index = 0; echoKeys != NULL && echoKeys[index] != NULL; index++) {
        value = NULL;
        if (starkbank_entity_string(patch, echoKeys[index], &value) != STARKCORE_OK
            || value == NULL) {
            continue;
        }
        if (query == NULL && starkcore_json_new_object(&query) != STARKCORE_OK) {
            starkcore_json_free(payload);
            return STARKCORE_ERROR_ARGUMENT;
        }
        if (starkcore_json_set_string(query, echoKeys[index], value) != STARKCORE_OK) {
            starkcore_json_free(payload);
            starkcore_json_free(query);
            return STARKCORE_ERROR_ARGUMENT;
        }
    }
    status = starkcore_rest_patch_id(starkbankClientCore(client), resource->name, id,
                                     payload, query, &reply, errors);
    starkcore_json_free(payload);
    starkcore_json_free(query);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, reply, out);
}

int starkbankVerbDeleteId(const starkbank_client *client, const starkbankResource *resource,
                          const char *id, starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *reply = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_rest_delete_id(starkbankClientCore(client), resource->name, id,
                                      NULL, &reply, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(resource, reply, out);
}

int starkbankVerbContent(const starkbank_client *client, const starkbankResource *resource,
                         const char *id, const char *subResourceName,
                         const char *queryKey, int queryValue, int queryMin, int queryMax,
                         unsigned char **out, size_t *outLen, starkbank_errors **errors)
{
    starkcore_json *query = NULL;
    int status;

    if (out == NULL || outLen == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    *outLen = 0;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (queryKey != NULL && queryValue != 0) {
        /* Zero means "send nothing and let the API apply its own default",
           which is not the same as sending the value zero. */
        if (queryValue < queryMin || queryValue > queryMax) {
            return STARKCORE_ERROR_ARGUMENT;
        }
        status = starkcore_json_new_object(&query);
        if (status != STARKCORE_OK) {
            return status;
        }
        status = starkcore_json_set_number(query, queryKey, (double)queryValue);
        if (status != STARKCORE_OK) {
            starkcore_json_free(query);
            return status;
        }
    }
    status = starkcore_rest_get_content(starkbankClientCore(client), resource->name, id,
                                        subResourceName, query, out, outLen, errors);
    starkcore_json_free(query);
    return status;
}

/*
 * The same shape as starkbankVerbContent, generalised to two optional string
 * query keys instead of one bounded integer: boleto.pdf's layout ("default"/
 * "booklet") and hiddenFields are why this exists, each omitted from the
 * request exactly like size 0 is - "send nothing and let the API default"
 * rather than a sentinel value. hiddenFields takes a caller-joined
 * comma-separated string rather than a list: core-c's own query encoder
 * (starkcore/utils/url.c) joins a JSON array with "," before percent-encoding
 * the whole value, so "a,b" here and ["a","b"] on sdk-python's side reach the
 * wire as the same bytes, and this keeps every parameter a plain const char *
 * that a binding generator already knows how to translate.
 */
int starkbankVerbContentQuery(const starkbank_client *client, const starkbankResource *resource,
                              const char *id, const char *subResourceName,
                              const char *stringKey1, const char *stringValue1,
                              const char *stringKey2, const char *stringValue2,
                              unsigned char **out, size_t *outLen, starkbank_errors **errors)
{
    starkcore_json *query = NULL;
    int status;
    int have1 = stringValue1 != NULL && stringValue1[0] != '\0';
    int have2 = stringValue2 != NULL && stringValue2[0] != '\0';

    if (out == NULL || outLen == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    *outLen = 0;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (have1 || have2) {
        status = starkcore_json_new_object(&query);
        if (status != STARKCORE_OK) {
            return status;
        }
    }
    if (have1) {
        status = starkcore_json_set_string(query, stringKey1, stringValue1);
        if (status != STARKCORE_OK) {
            starkcore_json_free(query);
            return status;
        }
    }
    if (have2) {
        status = starkcore_json_set_string(query, stringKey2, stringValue2);
        if (status != STARKCORE_OK) {
            starkcore_json_free(query);
            return status;
        }
    }
    status = starkcore_rest_get_content(starkbankClientCore(client), resource->name, id,
                                        subResourceName, query, out, outLen, errors);
    starkcore_json_free(query);
    return status;
}

int starkbankVerbSubResource(const starkbank_client *client, const starkbankResource *resource,
                             const char *id, const char *subResourceName, const char *tagName,
                             starkbank_entity **out, starkbank_errors **errors)
{
    starkcore_json *reply = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_rest_get_sub_resource(starkbankClientCore(client), resource->name, id,
                                             subResourceName, NULL, &reply, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    /* An unregistered tag leaves the entity untagged and permissive rather
       than failing: a sub-resource this build predates is still readable. */
    return wrapOwned(starkbankRegistryFind(tagName), reply, out);
}

/*
 * MerchantSession.purchase: rest.post_sub_resource, in full agreement with
 * core-c's starkcore_rest_post_sub_resource - POST to
 * endpoint(resource)/id/endpoint(subResourceName), body the dehydrated
 * entity, response unwrapped by the SUB resource's own singular name. Unlike
 * starkbankVerbCreateSub (CorporateCard.create: a literal subPath, no id, and
 * the OWNING resource's own name unwraps the reply) this is the shape a real
 * nested sub-resource create needs, and core-c already had it - no engine
 * change below this function was required, only this shim and the macro that
 * calls it.
 */
int starkbankVerbCreateSubResource(const starkbank_client *client,
                                   const starkbankResource *resource, const char *id,
                                   const char *subResourceName, const char *tagName,
                                   const starkbank_entity *entity, starkbank_entity **out,
                                   starkbank_errors **errors)
{
    const starkbankResource *subResource;
    starkcore_json *payload = NULL;
    starkcore_json *reply = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = checkVerb(client, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (id == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (starkbank_entity_json(entity) == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    subResource = starkbankRegistryFind(tagName);
    if (subResource != NULL && entity->resource != subResource) {
        return STARKBANK_ERROR_RESOURCE;
    }
    status = starkbankEntityDehydrate(entity, STARKBANK_FLAG_CREATE, &payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_rest_post_sub_resource(starkbankClientCore(client), resource->name, id,
                                              subResourceName, payload, &reply, errors);
    starkcore_json_free(payload);
    if (status != STARKCORE_OK) {
        return status;
    }
    return wrapOwned(subResource, reply, out);
}
