/*
 * python's generator over a listing endpoint. All the pagination arithmetic -
 * the per-page min(limit, 100), the remaining limit decremented by 100, the
 * cursor handling - is starkcore_stream's and is never reimplemented here.
 *
 * The yielded entity is borrowed until the next call, deliberately the same
 * rule as starkcore_stream_next, so one sentence covers both tiers.
 */

#include <stdlib.h>

#include "internal.h"

static int isIter(const starkbank_iter *iter)
{
    return iter != NULL && iter->magic == STARKBANK_MAGIC_ITER;
}

int starkbankIterNew(starkcore_stream *stream, const starkbankResource *resource,
                     starkbank_iter **out)
{
    starkbank_iter *iter;

    if (out == NULL) {
        starkcore_stream_free(stream);
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (stream == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    iter = (starkbank_iter *)calloc(1, sizeof(*iter));
    if (iter == NULL) {
        starkcore_stream_free(stream);
        return STARKCORE_ERROR_MEMORY;
    }
    iter->magic = STARKBANK_MAGIC_ITER;
    iter->stream = stream;
    iter->resource = resource;
    *out = iter;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_iter_next(starkbank_iter *iter,
    const starkbank_entity **out, starkbank_errors **errors)
{
    const starkcore_json *node = NULL;
    int status;

    if (errors != NULL) {
        *errors = NULL;
    }
    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (!isIter(iter)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    /* Freed before the next fetch, not after: the page it points into is the
       one starkcore is about to replace. */
    starkbank_entity_free(iter->current);
    iter->current = NULL;
    status = starkcore_stream_next(iter->stream, &node, errors);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (node == NULL) {
        return STARKCORE_OK;    /* exhausted: OK with a NULL entity, as python stops */
    }
    status = starkbankEntityWrap(iter->resource, node, NULL, &iter->current);
    if (status != STARKCORE_OK) {
        return status;
    }
    *out = iter->current;
    return STARKCORE_OK;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_iter_cursor(const starkbank_iter *iter)
{
    if (!isIter(iter)) {
        return NULL;
    }
    return starkcore_stream_cursor(iter->stream);
}

STARKBANK_API void STARKBANK_CALL starkbank_iter_free(starkbank_iter *iter)
{
    if (!isIter(iter)) {
        return;
    }
    starkbank_entity_free(iter->current);
    starkcore_stream_free(iter->stream);
    iter->magic = 0;
    free(iter);
}
