/*
 * The verb macros. A resource's .c file is its table plus a dozen of these,
 * and every one of them expands to a single call into starkc/verb.c.
 *
 * That is the whole shape on purpose. A macro body is hostile to a debugger:
 * -Werror diagnostics and breakpoints land here rather than on the line the
 * author wrote, so there is nothing in a body worth stopping on. The logic
 * lives in ordinary functions one step down, where a stack frame has a name
 * and `make expand R=<resource>` is only needed to read an argument list.
 *
 * ident is the public name stem, so STARKBANK_VERB_GET_ID(invoice_log) defines
 * starkbank_invoice_log_get and reads the table STARKBANK_RESOURCE declared as
 * starkbankTable_invoice_log. Every definition carries STARKBANK_API, without
 * which -fvisibility=hidden would hide the entry point from the shared build.
 */

#ifndef STARKBANK_VERBS_H
#define STARKBANK_VERBS_H

#include "internal.h"
#include "table.h"

#define STARKBANK_VERB_NEW(ident)                                                       \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_new(starkbank_entity **out)    \
    {                                                                                   \
        return starkbankEntityNew(&starkbankTable_##ident, out);                        \
    }

#define STARKBANK_VERB_PARAMS(ident)                                                        \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_params_new(starkbank_entity **out) \
    {                                                                                       \
        return starkbankEntityNew(&starkbankParams_##ident, out);                           \
    }

#define STARKBANK_VERB_POST_MULTI(ident)                                                 \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_create(                         \
        const starkbank_client *client, const starkbank_list *entities,                  \
        starkbank_list **out, starkbank_errors **errors)                                 \
    {                                                                                    \
        return starkbankVerbCreate(client, &starkbankTable_##ident, entities, out, errors); \
    }

/*
 * rest.post_single: one entity, sent as the body itself. Webhook is the only
 * bank resource with this shape, and the create signature differs from
 * POST_MULTI's by taking an entity rather than a list - which is the whole
 * point of it being a separate shape rather than a batch of one.
 */
#define STARKBANK_VERB_POST_SINGLE(ident)                                                \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_create(                         \
        const starkbank_client *client, const starkbank_entity *entity,                  \
        starkbank_entity **out, starkbank_errors **errors)                               \
    {                                                                                    \
        return starkbankVerbCreateSingle(client, &starkbankTable_##ident, entity,        \
                                         out, errors);                                   \
    }

/*
 * rest.post_raw to endpoint(resource) + "/" + subPath: one entity, no id in
 * the path, unwrapped by the resource's OWN singular name. CorporateCard.create
 * is the reason this exists - see starkbankVerbCreateSub's comment in verb.c
 * for why STARKBANK_VERB_POST_SINGLE and STARKBANK_VERB_SUB_RESOURCE each miss
 * by one detail.
 */
#define STARKBANK_VERB_POST_SINGLE_SUB(ident, subPath)                                  \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_create(                        \
        const starkbank_client *client, const starkbank_entity *entity,                 \
        starkbank_entity **out, starkbank_errors **errors)                              \
    {                                                                                    \
        return starkbankVerbCreateSub(client, &starkbankTable_##ident, subPath, entity, \
                                      out, errors);                                     \
    }

#define STARKBANK_VERB_GET_ID(ident)                                                     \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_get(                            \
        const starkbank_client *client, const char *id,                                  \
        starkbank_entity **out, starkbank_errors **errors)                               \
    {                                                                                    \
        return starkbankVerbGetId(client, &starkbankTable_##ident, id, out, errors);     \
    }

/* Balance has no id: python reads the listing endpoint and takes its head. */
#define STARKBANK_VERB_GET_FIRST(ident)                                                  \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_get(                            \
        const starkbank_client *client, starkbank_entity **out, starkbank_errors **errors) \
    {                                                                                    \
        return starkbankVerbGetFirst(client, &starkbankTable_##ident, out, errors);      \
    }

#define STARKBANK_VERB_QUERY(ident)                                                      \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_query(                          \
        const starkbank_client *client, const starkbank_entity *params, int limit,       \
        starkbank_iter **out)                                                            \
    {                                                                                    \
        return starkbankVerbQuery(client, &starkbankTable_##ident, params, limit, out);  \
    }

#define STARKBANK_VERB_PAGE(ident)                                                       \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_page(                           \
        const starkbank_client *client, const starkbank_entity *params,                  \
        starkbank_list **out, char **out_cursor, starkbank_errors **errors)              \
    {                                                                                    \
        return starkbankVerbPage(client, &starkbankTable_##ident, params, out,           \
                                 out_cursor, errors);                                    \
    }

#define STARKBANK_VERB_PATCH_ID(ident)                                                   \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_update(                         \
        const starkbank_client *client, const char *id, const starkbank_entity *patch,   \
        starkbank_entity **out, starkbank_errors **errors)                               \
    {                                                                                    \
        return starkbankVerbPatchId(client, &starkbankTable_##ident, id, patch, out, errors); \
    }

/* Same as STARKBANK_VERB_PATCH_ID, plus echoKeys (a NULL-terminated array of
   wire keys) mirrored into the URL's query string whenever patch carries
   them - the one shape Workspace.update needs and no other patchable
   resource does. See starkbankVerbPatchIdEcho's comment in internal.h. */
#define STARKBANK_VERB_PATCH_ID_ECHO(ident, echoKeys)                                    \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_update(                         \
        const starkbank_client *client, const char *id, const starkbank_entity *patch,   \
        starkbank_entity **out, starkbank_errors **errors)                               \
    {                                                                                    \
        return starkbankVerbPatchIdEcho(client, &starkbankTable_##ident, id, patch,       \
                                        echoKeys, out, errors);                          \
    }

#define STARKBANK_VERB_DELETE_ID(ident)                                                  \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_delete(                         \
        const starkbank_client *client, const char *id,                                  \
        starkbank_entity **out, starkbank_errors **errors)                               \
    {                                                                                    \
        return starkbankVerbDeleteId(client, &starkbankTable_##ident, id, out, errors);  \
    }

/* Raw bytes: a PDF, a receipt. The path segment is the verb's own name. */
#define STARKBANK_VERB_CONTENT(ident, verb)                                              \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_##verb(                         \
        const starkbank_client *client, const char *id,                                  \
        unsigned char **out, size_t *out_len, starkbank_errors **errors)                 \
    {                                                                                    \
        return starkbankVerbContent(client, &starkbankTable_##ident, id, #verb,          \
                                    NULL, 0, 0, 0, out, out_len, errors);                \
    }

/*
 * The same with one integer query parameter, bounded here rather than by the
 * API: starkbank_invoice_qrcode's size is 1..50 and 0 sends no size at all, so
 * the API applies its own default instead of being told a wrong one.
 */
#define STARKBANK_VERB_CONTENT_INT(ident, verb, queryKey, minimum, maximum)              \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_##verb(                         \
        const starkbank_client *client, const char *id, int value,                       \
        unsigned char **out, size_t *out_len, starkbank_errors **errors)                 \
    {                                                                                    \
        return starkbankVerbContent(client, &starkbankTable_##ident, id, #verb,          \
                                    queryKey, value, minimum, maximum, out, out_len, errors); \
    }

/*
 * The same with two optional string query keys instead of one bounded
 * integer, each sent only when non-empty. boleto.pdf's layout and
 * hiddenFields are the shape this exists for: sdk-python takes them as
 * ordinary keyword arguments and rest.get_content forwards them as query
 * params exactly like CONTENT_INT's size.
 *
 * hiddenFields is a comma-joined list on the wire either way - core-c's own
 * query encoder (starkcore/utils/url.c) joins a JSON array with "," and then
 * percent-encodes the whole value, so a caller-joined "a,b" string percent-
 * encodes to the identical bytes a two-element array would. A second string
 * parameter is therefore both the simplest ABI shape and the one every
 * binding generator already knows how to translate - no new pointer-array
 * parameter type, no per-resource hand-written function.
 */
#define STARKBANK_VERB_CONTENT_QUERY(ident, verb, stringKey1, stringKey2)                \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_##verb(                         \
        const starkbank_client *client, const char *id,                                  \
        const char *stringValue1, const char *stringValue2,                              \
        unsigned char **out, size_t *out_len, starkbank_errors **errors)                 \
    {                                                                                    \
        return starkbankVerbContentQuery(client, &starkbankTable_##ident, id, #verb,     \
                                         stringKey1, stringValue1, stringKey2,            \
                                         stringValue2, out, out_len, errors);             \
    }

#define STARKBANK_VERB_SUB_RESOURCE(ident, verb, subResourceName, tagName)               \
    STARKBANK_API int STARKBANK_CALL starkbank_##ident##_##verb(                         \
        const starkbank_client *client, const char *id,                                  \
        starkbank_entity **out, starkbank_errors **errors)                               \
    {                                                                                    \
        return starkbankVerbSubResource(client, &starkbankTable_##ident, id,             \
                                        subResourceName, tagName, out, errors);          \
    }

#endif /* STARKBANK_VERBS_H */
