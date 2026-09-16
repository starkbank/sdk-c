/*
 * The entity: one API object plus the tag naming its resource. There is no
 * hydration pass and no per-field strdup - the entity IS the document
 * starkcore handed back, which is what rest.c's own comment asks this tier to
 * be, and it is why a nested invoice inside a log needs no special case.
 *
 * Reads are permissive and writes are strict, and the asymmetry is the whole
 * point of the tables. A key the API added and this build's table lacks reads
 * back fine and bumps the unknown counter, so the gap becomes a red build in
 * this repo rather than an error in a caller's production path. A key written
 * that the table does not have is refused, because a payload key dropped in
 * silence moves money wrong.
 */

#include <stdlib.h>
#include <string.h>

#include "internal.h"

/* 2^53: above this a double stops being an exact integer, so an amount in
   cents stops being an amount. R$ 90 trillion; the check is nearly free. */
#define STARKBANK_EXACT_INTEGER_LIMIT 9007199254740992.0

static int isEntity(const starkbank_entity *entity)
{
    return entity != NULL && entity->magic == STARKBANK_MAGIC_ENTITY;
}

/* ---------------------------------------------------------- construction */

int starkbankEntityWrap(const starkbankResource *resource, const starkcore_json *json,
                        starkcore_json *owned, starkbank_entity **out)
{
    starkbank_entity *entity;
    unsigned char *block;

    if (out == NULL) {
        starkcore_json_free(owned);
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (json == NULL) {
        starkcore_json_free(owned);
        return STARKCORE_ERROR_ARGUMENT;
    }
    block = (unsigned char *)calloc(1, sizeof(starkbank_entity) + sizeof(starkbankChildren));
    if (block == NULL) {
        starkcore_json_free(owned);
        return STARKCORE_ERROR_MEMORY;
    }
    entity = (starkbank_entity *)block;
    entity->magic = STARKBANK_MAGIC_ENTITY;
    entity->resource = resource;
    entity->owned = owned;
    entity->json = json;
    entity->children = (starkbankChildren *)(block + sizeof(starkbank_entity));
    *out = entity;
    return STARKCORE_OK;
}

int starkbankEntityNew(const starkbankResource *resource, starkbank_entity **out)
{
    starkcore_json *document = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    status = starkcore_json_new_object(&document);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankEntityWrap(resource, document, document, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_object_new(starkbank_entity **out)
{
    return starkbankEntityNew(NULL, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_clone(const starkbank_entity *entity,
                                                        starkbank_entity **out)
{
    starkcore_json *copy = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (!isEntity(entity)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_json_clone(entity->json, &copy);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkbankEntityWrap(entity->resource, copy, copy, out);
}

STARKBANK_API void STARKBANK_CALL starkbank_entity_free(starkbank_entity *entity)
{
    int index;

    if (!isEntity(entity)) {
        return;
    }
    for (index = 0; index < entity->children->count; index++) {
        starkbank_entity_free(entity->children->items[index]);
    }
    free(entity->children->items);
    starkcore_json_free(entity->owned);
    /* Cleared before the block goes back, so a stale handle from an FFI host
       that kept an integer is rejected instead of dereferenced. It is not a
       licence to free twice: reading a freed block is still undefined. */
    entity->magic = 0;
    free(entity);
}

/* ------------------------------------------------------------- children */

/*
 * One wrapper per nested node, cached, so a read in a loop does not allocate
 * per iteration and so the handle the caller borrowed stays the same handle.
 */
static int childFor(const starkbank_entity *parent, const starkbankResource *resource,
                    const starkcore_json *node, const starkbank_entity **out)
{
    starkbankChildren *children = parent->children;
    starkbank_entity **grown;
    starkbank_entity *child = NULL;
    int capacity;
    int index;
    int status;

    for (index = 0; index < children->count; index++) {
        if (children->items[index]->json == node) {
            *out = children->items[index];
            return STARKCORE_OK;
        }
    }
    if (children->count == children->capacity) {
        capacity = children->capacity == 0 ? 4 : children->capacity * 2;
        grown = (starkbank_entity **)realloc(children->items,
                                             (size_t)capacity * sizeof(*grown));
        if (grown == NULL) {
            return STARKCORE_ERROR_MEMORY;
        }
        children->items = grown;
        children->capacity = capacity;
    }
    status = starkbankEntityWrap(resource, node, NULL, &child);
    if (status != STARKCORE_OK) {
        return status;
    }
    children->items[children->count] = child;
    children->count++;
    *out = child;
    return STARKCORE_OK;
}

/* A mutation invalidates every borrowed pointer into this document, which is
   ownership rule 1. Dropping the wrappers here is what makes that true rather
   than merely documented. */
static void childrenClear(starkbank_entity *entity)
{
    int index;

    for (index = 0; index < entity->children->count; index++) {
        starkbank_entity_free(entity->children->items[index]);
    }
    entity->children->count = 0;
}

/* --------------------------------------------------------------- reads */

/* What a JSON node can be read as when no table row declares its type. */
static int nodeReadableAs(const starkcore_json *node, int accessor)
{
    int type = starkcore_json_type(node);

    switch (accessor) {
    case STARKBANK_ACCESS_STRING:
    case STARKBANK_ACCESS_DATETIME:
        return type == STARKCORE_JSON_STRING;
    case STARKBANK_ACCESS_AMOUNT:
    case STARKBANK_ACCESS_NUMBER:
        return type == STARKCORE_JSON_NUMBER;
    case STARKBANK_ACCESS_BOOL:
        return type == STARKCORE_JSON_TRUE || type == STARKCORE_JSON_FALSE;
    case STARKBANK_ACCESS_LIST:
    case STARKBANK_ACCESS_STRINGS:
    case STARKBANK_ACCESS_ENTITIES:
        return type == STARKCORE_JSON_ARRAY;
    case STARKBANK_ACCESS_ENTITY:
        return type == STARKCORE_JSON_OBJECT;
    default:
        return 0;
    }
}

/*
 * One resolver for every reader. Order matters and is deliberate: a wrong
 * accessor is a wrong call whether the value is there or not, so TYPE beats
 * ABSENT; and a name in neither the table nor the document is FIELD, which is
 * how a typo reads, while a name the document has and the table lacks is
 * readable, which is how a field added upstream stays reachable.
 */
static int resolveRead(const starkbank_entity *entity, const char *field, int accessor,
                       const starkbankField **outField, const starkcore_json **outNode)
{
    const starkbankField *row;
    const starkcore_json *node;

    *outField = NULL;
    *outNode = NULL;
    if (!isEntity(entity) || field == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    row = starkbankFieldFind(entity->resource, field);
    node = starkcore_json_get(entity->json, field);
    if (row == NULL) {
        if (node == NULL) {
            return STARKBANK_ERROR_FIELD;
        }
        if (starkcore_json_type(node) == STARKCORE_JSON_NULL) {
            return STARKBANK_ERROR_ABSENT;
        }
        if (!nodeReadableAs(node, accessor)) {
            return STARKBANK_ERROR_TYPE;
        }
        *outNode = node;
        return STARKCORE_OK;
    }
    if (!starkbankTypeReadableAs(row->type, accessor)) {
        return STARKBANK_ERROR_TYPE;
    }
    if (node == NULL || starkcore_json_type(node) == STARKCORE_JSON_NULL) {
        return STARKBANK_ERROR_ABSENT;
    }
    *outField = row;
    *outNode = node;
    return STARKCORE_OK;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_entity_resource(const starkbank_entity *entity)
{
    if (!isEntity(entity) || entity->resource == NULL) {
        return NULL;
    }
    return entity->resource->name;
}

STARKBANK_API const char * STARKBANK_CALL starkbank_entity_id(const starkbank_entity *entity)
{
    if (!isEntity(entity)) {
        return NULL;
    }
    return starkcore_json_string(starkcore_json_get(entity->json, "id"));
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_has(const starkbank_entity *entity,
                                                      const char *field)
{
    const starkcore_json *node;

    if (!isEntity(entity) || field == NULL) {
        return 0;
    }
    node = starkcore_json_get(entity->json, field);
    return node != NULL && starkcore_json_type(node) != STARKCORE_JSON_NULL;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_string(const starkbank_entity *entity,
                                                         const char *field, const char **out)
{
    const starkbankField *row;
    const starkcore_json *node;
    const char *text;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, STARKBANK_ACCESS_STRING, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    text = starkcore_json_string(node);
    if (text == NULL) {
        return STARKBANK_ERROR_TYPE;
    }
    /* No mask check here on purpose. checks.py:33 redacts datetimes; a tax id
       arrives as "***.345.678-**" and that string IS the value the API sent,
       while a description containing an asterisk is nobody's redaction. */
    *out = text;
    return STARKCORE_OK;
}

static int readNumber(const starkbank_entity *entity, const char *field, int accessor,
                      double *out)
{
    const starkbankField *row;
    const starkcore_json *node;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, accessor, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkcore_json_number(node, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_amount(const starkbank_entity *entity,
                                                         const char *field, double *out)
{
    return readNumber(entity, field, STARKBANK_ACCESS_AMOUNT, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_number(const starkbank_entity *entity,
                                                         const char *field, double *out)
{
    return readNumber(entity, field, STARKBANK_ACCESS_NUMBER, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_bool(const starkbank_entity *entity,
                                                       const char *field, int *out)
{
    const starkbankField *row;
    const starkcore_json *node;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, STARKBANK_ACCESS_BOOL, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    return starkcore_json_bool(node, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_datetime(const starkbank_entity *entity,
    const char *field, int *year, int *month, int *day,
    int *hour, int *minute, int *second, int *out_has_time)
{
    const starkbankField *row;
    const starkcore_json *node;
    const char *text;
    int values[6];
    int hasTime = 0;
    int status;

    status = resolveRead(entity, field, STARKBANK_ACCESS_DATETIME, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    text = starkcore_json_string(node);
    if (text == NULL) {
        return STARKBANK_ERROR_TYPE;
    }
    if (starkcore_datetime_is_masked(text)) {
        return STARKBANK_ERROR_MASKED;
    }
    memset(values, 0, sizeof(values));
    status = starkcore_datetime_parse(text, &values[0], &values[1], &values[2],
                                      &values[3], &values[4], &values[5], &hasTime);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (year != NULL) { *year = values[0]; }
    if (month != NULL) { *month = values[1]; }
    if (day != NULL) { *day = values[2]; }
    if (hour != NULL) { *hour = values[3]; }
    if (minute != NULL) { *minute = values[4]; }
    if (second != NULL) { *second = values[5]; }
    if (out_has_time != NULL) { *out_has_time = hasTime; }
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_list_size(const starkbank_entity *entity,
                                                            const char *field, int *out)
{
    const starkbankField *row;
    const starkcore_json *node;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, STARKBANK_ACCESS_LIST, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    *out = starkcore_json_size(node);
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_list_string_at(const starkbank_entity *entity,
    const char *field, int index, const char **out)
{
    const starkbankField *row;
    const starkcore_json *node;
    const char *text;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, STARKBANK_ACCESS_STRINGS, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (index < 0 || index >= starkcore_json_size(node)) {
        return STARKBANK_ERROR_ABSENT;
    }
    text = starkcore_json_string(starkcore_json_at(node, index));
    if (text == NULL) {
        return STARKBANK_ERROR_TYPE;
    }
    *out = text;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_list_entity_at(const starkbank_entity *entity,
    const char *field, int index, const starkbank_entity **out)
{
    const starkbankField *row;
    const starkcore_json *node;
    const starkcore_json *element;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, STARKBANK_ACCESS_ENTITIES, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (index < 0 || index >= starkcore_json_size(node)) {
        return STARKBANK_ERROR_ABSENT;
    }
    element = starkcore_json_at(node, index);
    if (element == NULL || starkcore_json_type(element) != STARKCORE_JSON_OBJECT) {
        return STARKBANK_ERROR_TYPE;
    }
    return childFor(entity, starkbankFieldRef(entity->resource, row, entity->json),
                    element, out);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_entity(const starkbank_entity *entity,
    const char *field, const starkbank_entity **out)
{
    const starkbankField *row;
    const starkcore_json *node;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveRead(entity, field, STARKBANK_ACCESS_ENTITY, &row, &node);
    if (status != STARKCORE_OK) {
        return status;
    }
    return childFor(entity, starkbankFieldRef(entity->resource, row, entity->json), node, out);
}

STARKBANK_API const starkcore_json * STARKBANK_CALL starkbank_entity_json(
    const starkbank_entity *entity)
{
    if (!isEntity(entity)) {
        return NULL;
    }
    return entity->json;
}

/* ------------------------------------------------------- unknown fields */

static int unknownCount(const starkbankResource *resource, const starkcore_json *object);

static int unknownInValue(const starkbankResource *resource, const starkbankField *row,
                          const starkcore_json *parent, const starkcore_json *node)
{
    const starkbankResource *sub;
    int total = 0;
    int index;

    if (row->type != STARKBANK_FIELD_RESOURCE && row->type != STARKBANK_FIELD_LIST_RESOURCE) {
        return 0;   /* LIST_OBJECT and OBJECT are free-form by declaration */
    }
    sub = starkbankFieldRef(resource, row, parent);
    if (sub == NULL) {
        /* The key is known and its shape is not: one Event subscription this
           build predates counts as one gap, not as one per key inside it. */
        return 1;
    }
    if (row->type == STARKBANK_FIELD_RESOURCE) {
        return unknownCount(sub, node);
    }
    for (index = 0; index < starkcore_json_size(node); index++) {
        total += unknownCount(sub, starkcore_json_at(node, index));
    }
    return total;
}

static int unknownCount(const starkbankResource *resource, const starkcore_json *object)
{
    const starkbankField *row;
    const starkcore_json *node;
    const char *key;
    int total = 0;
    int index;

    if (resource == NULL || object == NULL
        || starkcore_json_type(object) != STARKCORE_JSON_OBJECT) {
        return 0;
    }
    for (index = 0; index < starkcore_json_size(object); index++) {
        key = starkcore_json_key_at(object, index);
        node = starkcore_json_at(object, index);
        row = starkbankFieldFind(resource, key);
        if (row == NULL) {
            total++;
            continue;
        }
        if (node == NULL || starkcore_json_type(node) == STARKCORE_JSON_NULL) {
            continue;
        }
        total += unknownInValue(resource, row, object, node);
    }
    return total;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_unknown_count(const starkbank_entity *entity)
{
    if (!isEntity(entity)) {
        return -1;
    }
    return unknownCount(entity->resource, entity->json);
}

/* -------------------------------------------------------------- writes */

/*
 * A params bag is not a create payload and is not a free-for-all. It accepts a
 * table field that is patchable, a table field the query list names, and a
 * query key with no wire type of its own - and nothing else, so a mistyped
 * filter fails here instead of being silently ignored by the API.
 */
static int resolveParamsWrite(const starkbankResource *resource, const char *field,
                              int accessor, const starkbankField **outField)
{
    const starkbankField *row = starkbankFieldFind(resource, field);
    int named = starkbankQueryKeyKnown(resource, field);

    if (row != NULL && ((row->flags & STARKBANK_FLAG_PATCH) != 0 || named)) {
        if (!starkbankTypeWritableAs(row->type, accessor)) {
            return STARKBANK_ERROR_TYPE;
        }
        *outField = row;
        return STARKCORE_OK;
    }
    if (named) {
        return STARKCORE_OK;    /* a filter name with no declared wire type */
    }
#ifdef STARKBANK_LOOSE_QUERY
    /* A wrong filter costs a retry where a wrong write costs money, so this
       loosening exists for queries and no equivalent exists for writes. */
    return STARKCORE_OK;
#else
    return STARKBANK_ERROR_FIELD;
#endif
}

static int resolveWrite(starkbank_entity *entity, const char *field, int accessor,
                        const starkbankField **outField)
{
    const starkbankField *row;

    *outField = NULL;
    if (!isEntity(entity) || field == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (entity->owned == NULL) {
        /* Borrowed from a list, a page or a parent. const says so in C; an FFI
           host has no const, so the engine refuses rather than writing into a
           document somebody else will free. */
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (entity->resource == NULL) {
        return STARKCORE_OK;    /* an untagged map: discounts, metadata */
    }
    if (entity->resource->isParams) {
        return resolveParamsWrite(entity->resource, field, accessor, outField);
    }
    row = starkbankFieldFind(entity->resource, field);
    if (row == NULL || (row->flags & STARKBANK_FLAG_CREATE) == 0) {
        return STARKBANK_ERROR_FIELD;
    }
    if (!starkbankTypeWritableAs(row->type, accessor)) {
        return STARKBANK_ERROR_TYPE;
    }
    *outField = row;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_string(starkbank_entity *entity,
    const char *field, const char *value)
{
    const starkbankField *row;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_STRING, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    childrenClear(entity);
    return starkcore_json_set_string(entity->owned, field, value);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_amount(starkbank_entity *entity,
    const char *field, double cents)
{
    const starkbankField *row;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_AMOUNT, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    /* NaN first: every comparison against it is false, so it walks through the
       range guard below and casting it to long long is undefined behaviour.
       Self-comparison detects it without <math.h> and without a C99-ism, which
       keeps this file compilable by the MSVC job. */
    if (cents != cents) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (cents < -STARKBANK_EXACT_INTEGER_LIMIT || cents > STARKBANK_EXACT_INTEGER_LIMIT) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    /* Rounding somebody's money in silence is worse than refusing it. */
    if (cents != (double)(long long)cents) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    childrenClear(entity);
    return starkcore_json_set_number(entity->owned, field, cents);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_number(starkbank_entity *entity,
    const char *field, double value)
{
    const starkbankField *row;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_NUMBER, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    /* A RATE field (Invoice.fine, Transfer.Rule.value) is money too. cJSON
       prints a non-finite number as JSON null, so accepting one here would
       report success and put the field on the wire with no value; the
       magnitude bound rejects the infinities, and nothing outside +/-2^53 is
       a rate. */
    if (value != value || value > STARKBANK_EXACT_INTEGER_LIMIT
            || value < -STARKBANK_EXACT_INTEGER_LIMIT) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    childrenClear(entity);
    return starkcore_json_set_number(entity->owned, field, value);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_bool(starkbank_entity *entity,
    const char *field, int value)
{
    const starkbankField *row;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_BOOL, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    childrenClear(entity);
    return starkcore_json_set_bool(entity->owned, field, value != 0);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_seconds(starkbank_entity *entity,
    const char *field, int seconds)
{
    const starkbankField *row;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_SECONDS, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    childrenClear(entity);
    return starkcore_json_set_number(entity->owned, field, (double)seconds);
}

/* core-c prints whatever it is handed, so the calendar check belongs here:
   validate at the boundary, trust the tiers below. */
static int validDate(int year, int month, int day)
{
    static const int lengths[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1) {
        return 0;
    }
    return day <= lengths[month - 1];
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_date(starkbank_entity *entity,
    const char *field, int year, int month, int day)
{
    const starkbankField *row;
    char *text = NULL;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_DATE, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    if (!validDate(year, month, day)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_date_format(year, month, day, &text);
    if (status != STARKCORE_OK) {
        return status;
    }
    childrenClear(entity);
    status = starkcore_json_set_string(entity->owned, field, text);
    starkcore_free(text);
    return status;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_datetime(starkbank_entity *entity,
    const char *field, int year, int month, int day, int hour, int minute, int second)
{
    const starkbankField *row;
    char *text = NULL;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_DATETIME, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    if (!validDate(year, month, day) || hour < 0 || hour > 23 || minute < 0 || minute > 59
        || second < 0 || second > 60) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_datetime_format(year, month, day, hour, minute, second, &text);
    if (status != STARKCORE_OK) {
        return status;
    }
    childrenClear(entity);
    status = starkcore_json_set_string(entity->owned, field, text);
    starkcore_free(text);
    return status;
}

/*
 * Appending rebuilds the array rather than mutating the one already in the
 * document: starkcore_json_get hands back a const node and casting the
 * qualifier away to reach inside another library's DOM is not a trade worth
 * making for a list that holds tags or rules. It is quadratic in the number of
 * appends to one field, over lists the API itself caps in the low hundreds.
 * core-c v2 owning this tier is where a mutable accessor belongs.
 */
static int appendToArray(starkbank_entity *entity, const char *field, starkcore_json *value)
{
    const starkcore_json *existing = starkcore_json_get(entity->owned, field);
    starkcore_json *array = NULL;
    starkcore_json *copy = NULL;
    int status;
    int index;

    status = starkcore_json_new_array(&array);
    if (status != STARKCORE_OK) {
        starkcore_json_free(value);
        return status;
    }
    if (existing != NULL && starkcore_json_type(existing) == STARKCORE_JSON_ARRAY) {
        for (index = 0; index < starkcore_json_size(existing); index++) {
            status = starkcore_json_clone(starkcore_json_at(existing, index), &copy);
            if (status != STARKCORE_OK) {
                starkcore_json_free(array);
                starkcore_json_free(value);
                return status;
            }
            status = starkcore_json_append(array, copy);
            if (status != STARKCORE_OK) {
                starkcore_json_free(array);
                starkcore_json_free(value);
                return status;
            }
        }
    }
    status = starkcore_json_append(array, value);
    if (status != STARKCORE_OK) {
        starkcore_json_free(array);
        return status;
    }
    childrenClear(entity);
    return starkcore_json_set_json(entity->owned, field, array);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_append_string(starkbank_entity *entity,
    const char *field, const char *value)
{
    const starkbankField *row;
    starkcore_json *holder = NULL;
    starkcore_json *element = NULL;
    const starkcore_json *source;
    int status = resolveWrite(entity, field, STARKBANK_ACCESS_STRINGS, &row);

    if (status != STARKCORE_OK) {
        return status;
    }
    if (value == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    /* There is no starkcore_json_new_string: a one-member object is the
       shortest honest way to get an owned string node out of the ABI. */
    status = starkcore_json_new_object(&holder);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_json_set_string(holder, "v", value);
    if (status != STARKCORE_OK) {
        starkcore_json_free(holder);
        return status;
    }
    source = starkcore_json_get(holder, "v");
    status = starkcore_json_clone(source, &element);
    starkcore_json_free(holder);
    if (status != STARKCORE_OK) {
        return status;
    }
    return appendToArray(entity, field, element);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_append_entity(starkbank_entity *entity,
    const char *field, starkbank_entity *value)
{
    const starkbankField *row;
    const starkbankResource *ref;
    starkcore_json *document = NULL;
    int status;

    if (!isEntity(value)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = resolveWrite(entity, field, STARKBANK_ACCESS_ENTITIES, &row);
    if (status != STARKCORE_OK) {
        starkbank_entity_free(value);
        return status;
    }
    if (row != NULL && row->type == STARKBANK_FIELD_LIST_RESOURCE) {
        ref = starkbankFieldRef(entity->resource, row, entity->json);
        if (ref != NULL && value->resource != ref) {
            starkbank_entity_free(value);
            return STARKBANK_ERROR_RESOURCE;
        }
    }
    document = value->owned;
    value->owned = NULL;                    /* moved into the parent document */
    if (document == NULL) {
        /* A borrowed entity - one read out of a list or a nested object - has
           nothing to move, so it is copied instead. */
        status = starkcore_json_clone(value->json, &document);
        if (status != STARKCORE_OK) {
            starkbank_entity_free(value);
            return status;
        }
    }
    starkbank_entity_free(value);
    return appendToArray(entity, field, document);
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_set_json_raw(starkbank_entity *entity,
    const char *field, const char *json_text)
{
    starkcore_json *parsed = NULL;
    int status;

    if (!isEntity(entity) || field == NULL || json_text == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (entity->owned == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_json_parse(json_text, strlen(json_text), &parsed);
    if (status != STARKCORE_OK) {
        return status;
    }
    childrenClear(entity);
    return starkcore_json_set_json(entity->owned, field, parsed);
}

/* ----------------------------------------------------------- dehydrate */

static int dehydrateObject(const starkbankResource *resource, const starkcore_json *object,
                           int flag, starkcore_json **out);

static int dehydrateValue(const starkbankResource *resource, const starkbankField *row,
                          const starkcore_json *parent, const starkcore_json *node,
                          int flag, starkcore_json **out)
{
    const starkbankResource *sub;
    starkcore_json *array = NULL;
    starkcore_json *element = NULL;
    int status;
    int index;

    sub = starkbankFieldRef(resource, row, parent);
    if (sub == NULL) {
        return starkcore_json_clone(node, out);
    }
    if (row->type == STARKBANK_FIELD_RESOURCE) {
        return dehydrateObject(sub, node, flag, out);
    }
    status = starkcore_json_new_array(&array);
    if (status != STARKCORE_OK) {
        return status;
    }
    for (index = 0; index < starkcore_json_size(node); index++) {
        status = dehydrateObject(sub, starkcore_json_at(node, index), flag, &element);
        if (status != STARKCORE_OK) {
            starkcore_json_free(array);
            return status;
        }
        status = starkcore_json_append(array, element);
        if (status != STARKCORE_OK) {
            starkcore_json_free(array);
            return status;
        }
    }
    *out = array;
    return STARKCORE_OK;
}

/*
 * A payload carries only what the flag admits. A key the table has without the
 * flag is refused rather than dropped, because a caller who set a return-only
 * field believes it was sent; a key the table does not have at all is passed
 * through, because set_json_raw is the documented way to send an API key this
 * build predates, and silently eating it would defeat the escape hatch.
 */
static int dehydrateObject(const starkbankResource *resource, const starkcore_json *object,
                           int flag, starkcore_json **out)
{
    const starkbankField *row;
    const starkcore_json *node;
    starkcore_json *result = NULL;
    starkcore_json *copy = NULL;
    const char *key;
    int status;
    int index;

    *out = NULL;
    if (object == NULL || starkcore_json_type(object) != STARKCORE_JSON_OBJECT) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = starkcore_json_new_object(&result);
    if (status != STARKCORE_OK) {
        return status;
    }
    for (index = 0; index < starkcore_json_size(object); index++) {
        key = starkcore_json_key_at(object, index);
        node = starkcore_json_at(object, index);
        row = starkbankFieldFind(resource, key);
        if (row != NULL && (row->flags & flag) == 0) {
            starkcore_json_free(result);
            return STARKBANK_ERROR_FIELD;
        }
        if (row == NULL && resource != NULL && resource->isParams
                && starkbankQueryKeyKnown(resource, key)) {
            /* A filter is not a patch: "limit" in an update body would be
               accepted by the API and mean nothing. */
            starkcore_json_free(result);
            return STARKBANK_ERROR_FIELD;
        }
        if (row == NULL) {
            status = starkcore_json_clone(node, &copy);
        }
        if (row != NULL) {
            status = dehydrateValue(resource, row, object, node, flag, &copy);
        }
        if (status != STARKCORE_OK) {
            starkcore_json_free(result);
            return status;
        }
        status = starkcore_json_set_json(result, key, copy);
        if (status != STARKCORE_OK) {
            starkcore_json_free(result);
            return status;
        }
    }
    *out = result;
    return STARKCORE_OK;
}

static int requiredMissing(const starkbankResource *resource, const starkcore_json *object,
                           const char **outKey)
{
    const starkbankField *row;
    const starkcore_json *node;
    const starkbankResource *sub;
    int index;
    int element;

    if (resource == NULL) {
        return 0;
    }
    for (index = 0; index < resource->fieldCount; index++) {
        row = &resource->fields[index];
        node = starkcore_json_get(object, row->key);
        if ((row->flags & STARKBANK_FLAG_REQUIRED) != 0
            && (node == NULL || starkcore_json_type(node) == STARKCORE_JSON_NULL)) {
            *outKey = row->key;
            return 1;
        }
        if (node == NULL) {
            continue;
        }
        sub = starkbankFieldRef(resource, row, object);
        if (sub == NULL) {
            continue;
        }
        if (row->type == STARKBANK_FIELD_RESOURCE) {
            if (requiredMissing(sub, node, outKey)) {
                return 1;
            }
            continue;
        }
        for (element = 0; element < starkcore_json_size(node); element++) {
            if (requiredMissing(sub, starkcore_json_at(node, element), outKey)) {
                return 1;
            }
        }
    }
    return 0;
}

int starkbankEntityRequiredMissing(const starkbank_entity *entity, const char **outKey)
{
    const char *key = NULL;

    if (!isEntity(entity)) {
        return 0;
    }
    if (!requiredMissing(entity->resource, entity->json, &key)) {
        return 0;
    }
    if (outKey != NULL) {
        *outKey = key;
    }
    return 1;
}

int starkbankEntityDehydrate(const starkbank_entity *entity, int flag, starkcore_json **out)
{
    const char *key = NULL;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (!isEntity(entity)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (flag == STARKBANK_FLAG_CREATE && requiredMissing(entity->resource, entity->json, &key)) {
        return STARKBANK_ERROR_FIELD;
    }
    return dehydrateObject(entity->resource, entity->json, flag, out);
}

/* ---------------------------------------------------------------- dump */

/*
 * Sorted keys and table-filtered, so two runs on two machines produce the same
 * bytes and a hydration golden can be compared against python's
 * json.dumps(api.api_json(obj), sort_keys=True) directly. Unknown keys are
 * left out on purpose: they are what unknown_count reports, and folding them
 * in here would make every new upstream field look like a hydration diff.
 */
static int sortedClone(const starkbankResource *resource, const starkcore_json *node,
                       starkcore_json **out);

static int sortedCloneObject(const starkbankResource *resource, const starkcore_json *object,
                             starkcore_json **out)
{
    const starkbankField *row;
    const starkcore_json *node;
    starkcore_json *result = NULL;
    starkcore_json *copy = NULL;
    const char *key;
    int *order;
    int count = starkcore_json_size(object);
    int used = 0;
    int index;
    int scan;
    int slot;
    int status;

    status = starkcore_json_new_object(&result);
    if (status != STARKCORE_OK) {
        return status;
    }
    if (count <= 0) {
        *out = result;
        return STARKCORE_OK;
    }
    order = (int *)malloc((size_t)count * sizeof(*order));
    if (order == NULL) {
        starkcore_json_free(result);
        return STARKCORE_ERROR_MEMORY;
    }
    for (index = 0; index < count; index++) {
        key = starkcore_json_key_at(object, index);
        if (resource != NULL && starkbankFieldFind(resource, key) == NULL) {
            continue;
        }
        /* python's api_json drops a None, and every reader here reports a null
           as ABSENT. Keeping it would make a field neither side carries read as
           a hydration diff. */
        if (starkcore_json_type(starkcore_json_at(object, index)) == STARKCORE_JSON_NULL) {
            continue;
        }
        slot = used;
        for (scan = 0; scan < used; scan++) {
            if (strcmp(key, starkcore_json_key_at(object, order[scan])) < 0) {
                slot = scan;
                break;
            }
        }
        for (scan = used; scan > slot; scan--) {
            order[scan] = order[scan - 1];
        }
        order[slot] = index;
        used++;
    }
    for (index = 0; index < used; index++) {
        key = starkcore_json_key_at(object, order[index]);
        node = starkcore_json_at(object, order[index]);
        row = starkbankFieldFind(resource, key);
        status = sortedClone(starkbankFieldRef(resource, row, object), node, &copy);
        if (status == STARKCORE_OK) {
            status = starkcore_json_set_json(result, key, copy);
        }
        if (status != STARKCORE_OK) {
            free(order);
            starkcore_json_free(result);
            return status;
        }
    }
    free(order);
    *out = result;
    return STARKCORE_OK;
}

static int sortedClone(const starkbankResource *resource, const starkcore_json *node,
                       starkcore_json **out)
{
    starkcore_json *array = NULL;
    starkcore_json *element = NULL;
    int status;
    int index;

    if (node == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    if (starkcore_json_type(node) == STARKCORE_JSON_OBJECT) {
        return sortedCloneObject(resource, node, out);
    }
    if (starkcore_json_type(node) != STARKCORE_JSON_ARRAY) {
        return starkcore_json_clone(node, out);
    }
    status = starkcore_json_new_array(&array);
    if (status != STARKCORE_OK) {
        return status;
    }
    for (index = 0; index < starkcore_json_size(node); index++) {
        status = sortedClone(resource, starkcore_json_at(node, index), &element);
        if (status == STARKCORE_OK) {
            status = starkcore_json_append(array, element);
        }
        if (status != STARKCORE_OK) {
            starkcore_json_free(array);
            return status;
        }
    }
    *out = array;
    return STARKCORE_OK;
}

STARKBANK_API int STARKBANK_CALL starkbank_entity_dump(const starkbank_entity *entity,
    char **out, size_t *out_len)
{
    starkcore_json *sorted = NULL;
    int status;

    if (out == NULL) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    *out = NULL;
    if (out_len != NULL) {
        *out_len = 0;
    }
    if (!isEntity(entity)) {
        return STARKCORE_ERROR_ARGUMENT;
    }
    status = sortedClone(entity->resource, entity->json, &sorted);
    if (status != STARKCORE_OK) {
        return status;
    }
    status = starkcore_json_print(sorted, out, out_len);
    starkcore_json_free(sorted);
    if (status != STARKCORE_OK) {
        *out = NULL;
    }
    return status;
}
