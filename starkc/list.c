/*
 * An owned array of entities: a create batch on the way out, a page on the way
 * back. The list owns what it holds, which is ownership rule 3 and the one an
 * FFI host is most likely to get wrong in the other direction.
 *
 * A page keeps the whole response document and its entities borrow elements of
 * it, so a hundred-object page costs one allocation and a hundred wrappers
 * rather than a hundred deep copies. The document is therefore freed last.
 */

#include <stdlib.h>

#include "internal.h"

static int isList(const starkbank_list *list)
{
    return list != NULL && list->magic == STARKBANK_MAGIC_LIST;
}

STARKBANK_API int STARKBANK_CALL starkbank_list_new(starkbank_list **out)
{
    starkbank_list *list;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    list = (starkbank_list *)calloc(1, sizeof(*list));
    if (list == NULL) {
        return STARKCORE_ERROR_MEMORY;
    }
    list->magic = STARKBANK_MAGIC_LIST;
    *out = list;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_list_append(starkbank_list *list,
                                                       starkbank_entity *entity)
{
    starkbank_entity **grown;
    int capacity;

    if (entity == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    /* Ownership rule 3 has no exceptions: the header says append consumes the
       entity on failure as well as on success, so a bad list must consume it
       too. A caller who follows the header would otherwise leak here, and one
       who compensates would double-free on the realloc path below.
       starkbank_entity_free tag-checks, so a foreign pointer is still safe. */
    if (!isList(list)) {
        starkbank_entity_free(entity);
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (list->count == list->capacity) {
        capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        grown = (starkbank_entity **)realloc(list->items, (size_t)capacity * sizeof(*grown));
        if (grown == NULL) {
            starkbank_entity_free(entity);
            return STARKCORE_ERROR_MEMORY;
        }
        list->items = grown;
        list->capacity = capacity;
    }
    list->items[list->count] = entity;
    list->count++;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_list_count(const starkbank_list *list)
{
    if (!isList(list)) {
        return -1;
    }
    return list->count;
}

STARKBANK_API const starkbank_entity * STARKBANK_CALL starkbank_list_at(
    const starkbank_list *list, int index)
{
    if (!isList(list) || index < 0 || index >= list->count) {
        return NULL;
    }
    return list->items[index];
}

STARKBANK_API void STARKBANK_CALL starkbank_list_free(starkbank_list *list)
{
    int index;

    if (!isList(list)) {
        return;
    }
    for (index = 0; index < list->count; index++) {
        starkbank_entity_free(list->items[index]);
    }
    free(list->items);
    /* Last: the entities above borrowed their nodes from this document. */
    starkcore_json_free(list->document);
    list->magic = 0;
    free(list);
}

int starkbankListFromJson(const starkbankResource *resource, starkcore_json *array,
                          starkbank_list **out)
{
    starkbank_list *list = NULL;
    starkbank_entity *entity = NULL;
    int status;
    int index;

    if (out == NULL) {
        starkcore_json_free(array);
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (array == NULL || starkcore_json_type(array) != STARKCORE_JSON_ARRAY) {
        starkcore_json_free(array);
        return STARKCORE_ERROR_ENCODING;
    }
    status = starkbank_list_new(&list);
    if (status != STARKCORE_OK) {
        starkcore_json_free(array);
        return status;
    }
    list->document = array;
    for (index = 0; index < starkcore_json_size(array); index++) {
        status = starkbankEntityWrap(resource, starkcore_json_at(array, index), NULL, &entity);
        if (status != STARKCORE_OK) {
            starkbank_list_free(list);
            return status;
        }
        status = starkbank_list_append(list, entity);
        if (status != STARKCORE_OK) {
            starkbank_list_free(list);
            return status;
        }
    }
    *out = list;
    return STARKCORE_OK;
}
