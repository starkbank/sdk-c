/*
 * Field lookup and the type matrix. Everything the tables mean, in one place,
 * so entity.c holds no knowledge of what a RATE is.
 */

#include <string.h>

#include "internal.h"

/*
 * Pagination is core's business, not any resource's, so these two are legal on
 * every params bag whether or not a table remembered to list them. Forty-three
 * chances to forget "cursor" is forty-three bugs waiting.
 */
static const char *const paginationKeys[] = { "limit", "cursor", NULL };

/*
 * Linear over at most 38 rows, length first so the common miss costs one
 * integer compare rather than a strcmp. If a POS profile ever shows this, the
 * fix is an interned field id per resource, and it changes no ABI - which is
 * the whole reason every read is funnelled through here.
 */
const starkbankField * starkbankFieldFind(const starkbankResource *resource, const char *key)
{
    const starkbankField *field;
    size_t length;
    int index;

    if (resource == NULL || key == NULL) {
        return NULL;
    }
    length = strlen(key);
    for (index = 0; index < resource->fieldCount; index++) {
        field = &resource->fields[index];
        if (strlen(field->key) != length) {
            continue;
        }
        if (memcmp(field->key, key, length) == 0) {
            return field;
        }
    }
    return NULL;
}

int starkbankQueryKeyKnown(const starkbankResource *resource, const char *key)
{
    int index;

    if (key == NULL) {
        return 0;
    }
    for (index = 0; paginationKeys[index] != NULL; index++) {
        if (strcmp(paginationKeys[index], key) == 0) {
            return 1;
        }
    }
    if (resource == NULL || resource->queryKeys == NULL) {
        return 0;
    }
    for (index = 0; resource->queryKeys[index] != NULL; index++) {
        if (strcmp(resource->queryKeys[index], key) == 0) {
            return 1;
        }
    }
    return 0;
}

const starkbankResource * starkbankFieldRef(const starkbankResource *resource,
                                            const starkbankField *field,
                                            const starkcore_json *object)
{
    if (field == NULL) {
        return NULL;
    }
    if (field->type != STARKBANK_FIELD_RESOURCE && field->type != STARKBANK_FIELD_LIST_RESOURCE) {
        return NULL;
    }
    if (field->ref != NULL) {
        return starkbankRegistryFind(field->ref);
    }
    if (resource != NULL && resource->refResolve != NULL) {
        return resource->refResolve(object, field->key);
    }
    return NULL;
}

/*
 * The read matrix. A RATE, a SECONDS and a NUMBER all come back through
 * entity_number because all three are plain JSON numbers a caller does
 * arithmetic on; an AMOUNT does not, because integer cents and a percentage
 * being interchangeable is how money gets multiplied by 2.5.
 */
int starkbankTypeReadableAs(int fieldType, int accessor)
{
    switch (fieldType) {
    case STARKBANK_FIELD_STRING:
        return accessor == STARKBANK_ACCESS_STRING;
    case STARKBANK_FIELD_AMOUNT:
        return accessor == STARKBANK_ACCESS_AMOUNT;
    case STARKBANK_FIELD_RATE:
    case STARKBANK_FIELD_SECONDS:
    case STARKBANK_FIELD_NUMBER:
        return accessor == STARKBANK_ACCESS_NUMBER;
    case STARKBANK_FIELD_BOOL:
        return accessor == STARKBANK_ACCESS_BOOL;
    case STARKBANK_FIELD_DATE:
    case STARKBANK_FIELD_DATETIME:
    case STARKBANK_FIELD_DATE_OR_DATETIME:
        return accessor == STARKBANK_ACCESS_DATETIME;
    case STARKBANK_FIELD_LIST_STRING:
        return accessor == STARKBANK_ACCESS_LIST || accessor == STARKBANK_ACCESS_STRINGS;
    case STARKBANK_FIELD_LIST_OBJECT:
    case STARKBANK_FIELD_LIST_RESOURCE:
        return accessor == STARKBANK_ACCESS_LIST || accessor == STARKBANK_ACCESS_ENTITIES;
    case STARKBANK_FIELD_RESOURCE:
    case STARKBANK_FIELD_OBJECT:
        return accessor == STARKBANK_ACCESS_ENTITY;
    default:
        return 0;
    }
}

/*
 * The write matrix, and it is not the read matrix transposed. A DATE takes
 * set_date and a DATETIME takes set_datetime because the two produce different
 * wire strings with different meanings - a date in Invoice.due is a scheduled
 * invoice - and DATE_OR_DATETIME is the only field where the caller's choice
 * of writer IS the semantic. Pushing that onto caller string formatting was
 * rejected outright.
 */
int starkbankTypeWritableAs(int fieldType, int accessor)
{
    switch (fieldType) {
    case STARKBANK_FIELD_STRING:
        return accessor == STARKBANK_ACCESS_STRING;
    case STARKBANK_FIELD_AMOUNT:
        return accessor == STARKBANK_ACCESS_AMOUNT;
    case STARKBANK_FIELD_RATE:
    case STARKBANK_FIELD_NUMBER:
        return accessor == STARKBANK_ACCESS_NUMBER;
    case STARKBANK_FIELD_SECONDS:
        return accessor == STARKBANK_ACCESS_SECONDS;
    case STARKBANK_FIELD_BOOL:
        return accessor == STARKBANK_ACCESS_BOOL;
    case STARKBANK_FIELD_DATE:
        return accessor == STARKBANK_ACCESS_DATE;
    case STARKBANK_FIELD_DATETIME:
        return accessor == STARKBANK_ACCESS_DATETIME;
    case STARKBANK_FIELD_DATE_OR_DATETIME:
        return accessor == STARKBANK_ACCESS_DATE || accessor == STARKBANK_ACCESS_DATETIME;
    case STARKBANK_FIELD_LIST_STRING:
        return accessor == STARKBANK_ACCESS_STRINGS;
    case STARKBANK_FIELD_LIST_OBJECT:
    case STARKBANK_FIELD_LIST_RESOURCE:
        return accessor == STARKBANK_ACCESS_ENTITIES;
    default:
        return 0;
    }
}
